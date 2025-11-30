#include "download_manager.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sys/stat.h>
#include <unistd.h>

DownloadManager::DownloadManager(const FilterConfig& filter_config, int max_threads)
    : url_filter(filter_config), stop_flag(false), active_downloads(0) {
    thread_pool = new ThreadPool(max_threads);
    state_manager = nullptr;
}

DownloadManager::~DownloadManager() {
    stop_download();
    delete thread_pool;
    if (state_manager) {
        delete state_manager;
    }
}

std::string DownloadManager::generate_local_path(const std::string& url, 
                                                const std::string& output_dir) {
    size_t pos = url.find("://");
    if (pos == std::string::npos) return "";
    
    std::string path = url.substr(pos + 3);
    std::replace(path.begin(), path.end(), '/', '_');
    std::replace(path.begin(), path.end(), ':', '_');
    std::replace(path.begin(), path.end(), '?', '_');
    std::replace(path.begin(), path.end(), '&', '_');
    
    if (path.empty() || path.back() == '_') {
        path += "index.html";
    }
    
    return output_dir + "/" + path;
}

void DownloadManager::show_progress(const DownloadItem& item) {
    if (progress_callback) {
        progress_callback(item);
    }
    
    // 顯示在終端機
    std::cout << "\r[";
    switch (item.status) {
        case DownloadStatus::PENDING: std::cout << "  等待  "; break;
        case DownloadStatus::DOWNLOADING: std::cout << "下載中 "; break;
        case DownloadStatus::COMPLETED: std::cout << " 完成  "; break;
        case DownloadStatus::FAILED: std::cout << " 失敗  "; break;
        case DownloadStatus::SKIPPED: std::cout << " 跳過  "; break;
    }
    std::cout << "] " << item.url.substr(0, 50);
    
    if (item.status == DownloadStatus::DOWNLOADING && item.total_size > 0) {
        double progress = (double)item.downloaded_size / item.total_size * 100;
        std::cout << " " << std::fixed << std::setprecision(1) << progress << "%";
        std::cout << " " << item.speed << " KB/s";
    }
    
    std::cout << std::string(20, ' ') << std::flush;
}

void DownloadManager::download_file(const std::string& url, const std::string& output_dir,
                                   int depth, const std::string& current_host) {
    if (stop_flag) return;
    
    active_downloads++;
    
    // 檢查是否應該下載
    std::string url_host = URLFilter::extract_host(url);
    std::string check_host = url_host.empty() ? current_host : url_host;
    
    if (!url_filter.should_download(url, depth, check_host)) {
        active_downloads--;
        return;
    }
    
    // 生成本地路徑
    std::string local_path = generate_local_path(url, output_dir);
    
    // 檢查是否已經下載過
    DownloadItem* existing_item = state_manager->get_item(url);
    if (existing_item && existing_item->status == DownloadStatus::COMPLETED) {
        std::ifstream test_file(local_path);
        if (test_file.good()) {
            // 已經成功下載，跳過
            active_downloads--;
            return;
        }
    }
    
    // 建立下載項目
    DownloadItem item;
    item.url = url;
    item.local_path = local_path;
    item.status = DownloadStatus::DOWNLOADING;
    item.start_time = std::chrono::steady_clock::now();
    
    state_manager->add_item(item);
    state_manager->save_state();
    
    // 確保目錄存在
    size_t last_slash = local_path.find_last_of('/');
    if (last_slash != std::string::npos) {
        std::string dir = local_path.substr(0, last_slash);
        mkdir(dir.c_str(), 0755);
    }
    
    // 發送HEAD請求取得檔案資訊
    HTTPResponse head_response;
    if (http_client.head(url, head_response)) {
        item.total_size = head_response.content_length;
        
        // 檢查檔案大小
        if (item.total_size > 0 && !url_filter.check_file_size(item.total_size)) {
            item.status = DownloadStatus::SKIPPED;
            state_manager->update_item(url, DownloadStatus::SKIPPED);
            active_downloads--;
            return;
        }
    }
    
    // 發送GET請求下載檔案
    HTTPResponse get_response;
    if (http_client.get(url, get_response)) {
        if (get_response.status_code == 200) {
            // 儲存檔案
            std::ofstream file(local_path, std::ios::binary);
            if (file.is_open()) {
                file.write(get_response.body.data(), get_response.body.size());
                file.close();
                
                item.downloaded_size = get_response.body.size();
                item.status = DownloadStatus::COMPLETED;
                item.end_time = std::chrono::steady_clock::now();
                
                // 計算速度
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                    item.end_time - item.start_time).count();
                if (duration > 0) {
                    item.speed = (double)item.downloaded_size / duration / 1024.0;
                }
                
                state_manager->update_item(url, DownloadStatus::COMPLETED, 
                                         item.downloaded_size, item.speed);
                
                // 如果是HTML檔案，處理其中的連結
                if (local_path.find(".html") != std::string::npos || 
                    local_path.find(".htm") != std::string::npos) {
                    std::string html_content(get_response.body.begin(), get_response.body.end());
                    process_html(html_content, url, output_dir, depth + 1, check_host);
                }
            } else {
                item.status = DownloadStatus::FAILED;
                item.error_code = 2; // 檔案寫入失敗
                state_manager->update_item(url, DownloadStatus::FAILED);
            }
        } else {
            item.status = DownloadStatus::FAILED;
            item.error_code = get_response.status_code;
            state_manager->update_item(url, DownloadStatus::FAILED);
        }
    } else {
        item.status = DownloadStatus::FAILED;
        item.error_code = 1; // 網路錯誤
        state_manager->update_item(url, DownloadStatus::FAILED);
    }
    
    show_progress(item);
    state_manager->save_state();
    active_downloads--;
}

void DownloadManager::process_html(const std::string& html, const std::string& base_url,
                                  const std::string& output_dir, int depth,
                                  const std::string& current_host) {
    if (depth > url_filter.config.max_depth) return;
    
    // 提取所有連結
    std::vector<std::string> links = html_parser.extract_links(html, base_url);
    std::vector<std::string> resources = html_parser.extract_resources(html, base_url);
    
    // 合併連結
    links.insert(links.end(), resources.begin(), resources.end());
    
    // 去重
    std::set<std::string> unique_links(links.begin(), links.end());
    
    // 加入下載佇列
    for (const std::string& link : unique_links) {
        if (stop_flag) break;
        
        thread_pool->enqueue([this, link, output_dir, depth, current_host] {
            this->download_file(link, output_dir, depth, current_host);
        });
    }
}

void DownloadManager::start_download(const std::string& url, const std::string& output_dir) {
    // 建立輸出目錄
    mkdir(output_dir.c_str(), 0755);
    
    // 初始化狀態管理器
    if (state_manager) {
        delete state_manager;
    }
    state_manager = new StateManager(output_dir);
    
    // 載入之前的狀態
    state_manager->load_state();
    
    // 設定基礎主機
    std::string base_host = URLFilter::extract_host(url);
    url_filter.config.base_host = base_host;
    
    // 開始下載
    stop_flag = false;
    
    // 加入初始URL到佇列
    thread_pool->enqueue([this, url, output_dir] {
        this->download_file(url, output_dir, 0, this->url_filter.config.base_host);
    });
}

void DownloadManager::stop_download() {
    stop_flag = true;
    wait_for_completion();
}

void DownloadManager::set_progress_callback(std::function<void(const DownloadItem&)> callback) {
    progress_callback = callback;
}

void DownloadManager::wait_for_completion() {
    while (active_downloads > 0 || thread_pool->get_active_tasks() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void DownloadManager::show_statistics() {
    int total_files, completed_files, failed_files;
    long total_size, downloaded_size;
    double total_time;
    
    state_manager->get_statistics(total_files, completed_files, failed_files,
                                 total_size, downloaded_size, total_time);
    
    std::cout << "\n\n========== 下載統計 ==========" << std::endl;
    std::cout << "總檔案數: " << total_files << std::endl;
    std::cout << "已完成: " << completed_files << std::endl;
    std::cout << "失敗: " << failed_files << std::endl;
    std::cout << "總大小: " << std::fixed << std::setprecision(2) 
              << total_size / 1024.0 / 1024.0 << " MB" << std::endl;
    std::cout << "已下載: " << downloaded_size / 1024.0 / 1024.0 << " MB" << std::endl;
    std::cout << "總時間: " << total_time << " 秒" << std::endl;
    
    if (total_time > 0) {
        double avg_speed = downloaded_size / total_time / 1024.0;
        std::cout << "平均速度: " << avg_speed << " KB/s" << std::endl;
    }
    std::cout << "==============================" << std::endl;
}
