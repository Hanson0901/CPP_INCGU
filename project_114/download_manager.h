#ifndef DOWNLOAD_MANAGER_H
#define DOWNLOAD_MANAGER_H

#include "http_client.h"
#include "html_parser.h"
#include "url_filter.h"
#include "thread_pool.h"
#include "state_manager.h"
#include "download_state.h"
#include <atomic>
#include <functional>

class DownloadManager {
private:
    HTTPClient http_client;
    HTMLParser html_parser;
    URLFilter url_filter;
    ThreadPool* thread_pool;
    StateManager* state_manager;
    
    std::atomic<bool> stop_flag;
    std::atomic<int> active_downloads;
    std::function<void(const DownloadItem&)> progress_callback;
    
    // 下載單個檔案
    void download_file(const std::string& url, const std::string& output_dir,
                      int depth, const std::string& current_host);
    
    // 處理HTML頁面（提取連結）
    void process_html(const std::string& html, const std::string& base_url,
                     const std::string& output_dir, int depth, const std::string& current_host);
    
    // 生成本地檔案路徑
    std::string generate_local_path(const std::string& url, const std::string& output_dir);
    
    // 顯示進度
    void show_progress(const DownloadItem& item);
    
public:
    DownloadManager(const FilterConfig& filter_config, int max_threads);
    ~DownloadManager();
    
    // 開始下載
    void start_download(const std::string& url, const std::string& output_dir);
    
    // 停止下載
    void stop_download();
    
    // 設定進度回呼函式
    void set_progress_callback(std::function<void(const DownloadItem&)> callback);
    
    // 等待所有下載完成
    void wait_for_completion();
    
    // 顯示統計資訊
    void show_statistics();
};

#endif
