#include <iostream>
#include <string>
#include <vector>
#include <signal.h>
#include <sstream>
#include "download_manager.h"
#include "url_filter.h"

// 全域變數用於信號處理
DownloadManager* g_download_manager = nullptr;

void signal_handler(int signal) {
    if (signal == SIGINT && g_download_manager) {
        std::cout << "\n\n收到中斷信號，正在停止下載..." << std::endl;
        g_download_manager->stop_download();
        exit(0);
    }
}

void print_usage(const char* program_name) {
    std::cout << "使用方式: " << program_name << " [選項]\n\n";
    std::cout << "選項:\n";
    std::cout << "  -u, --url <URL>          要下載的網站URL (必需)\n";
    std::cout << "  -o, --output <目錄>       輸出目錄 (預設: ./download)\n";
    std::cout << "  -d, --depth <深度>        遞迴深度 (預設: 0)\n";
    std::cout << "  -e, --external           下載外部網站內容\n";
    std::cout << "  -t, --threads <數量>      同時下載線程數 (預設: 4)\n";
    std::cout << "  -f, --filter <類型>       檔案類型過濾 (例: jpg,png,css)\n";
    std::cout << "  -m, --min-size <大小>     最小檔案大小 (KB)\n";
    std::cout << "  -x, --max-size <大小>     最大檔案大小 (KB)\n";
    std::cout << "  -i, --interactive        互動模式\n";
    std::cout << "  -h, --help               顯示說明\n";
    std::cout << "\n範例:\n";
    std::cout << "  " << program_name << " -u http://example.com -o ./output -d 2 -t 8\n";
    std::cout << "  " << program_name << " -i\n";
}

void interactive_mode() {
    std::string url, output_dir;
    int depth = 0, threads = 4;
    bool external = false;
    std::string filter_types;
    long min_size = 0, max_size = -1;
    
    std::cout << "=== 網站下載器 - 互動模式 ===\n" << std::endl;
    
    std::cout << "請輸入網站URL: ";
    std::getline(std::cin, url);
    
    std::cout << "請輸入輸出目錄 (預設: ./download): ";
    std::getline(std::cin, output_dir);
    if (output_dir.empty()) output_dir = "./download";
    
    std::cout << "請輸入遞迴深度 (0-5, 預設: 0): ";
    std::string depth_str;
    std::getline(std::cin, depth_str);
    if (!depth_str.empty()) depth = std::stoi(depth_str);
    
    std::cout << "是否下載外部網站 (y/N): ";
    std::string external_str;
    std::getline(std::cin, external_str);
    external = (external_str == "y" || external_str == "Y");
    
    std::cout << "請輸入同時下載線程數 (1-20, 預設: 4): ";
    std::string threads_str;
    std::getline(std::cin, threads_str);
    if (!threads_str.empty()) threads = std::stoi(threads_str);
    
    std::cout << "請輸入檔案類型過濾 (例: jpg,png,css, 留空表示全部): ";
    std::getline(std::cin, filter_types);
    
    std::cout << "請輸入最小檔案大小 (KB, 0表示不限): ";
    std::string min_size_str;
    std::getline(std::cin, min_size_str);
    if (!min_size_str.empty()) min_size = std::stol(min_size_str);
    
    std::cout << "請輸入最大檔案大小 (KB, 0表示不限): ";
    std::string max_size_str;
    std::getline(std::cin, max_size_str);
    if (!max_size_str.empty()) {
        long temp = std::stol(max_size_str);
        if (temp > 0) max_size = temp;
    }
    
    // 設定篩選器
    FilterConfig filter_config;
    filter_config.max_depth = depth;
    filter_config.download_external = external;
    filter_config.base_host = URLFilter::extract_host(url);
    filter_config.min_file_size = min_size * 1024; // 轉換為bytes
    
    if (max_size > 0) {
        filter_config.max_file_size = max_size * 1024; // 轉換為bytes
    }
    
    if (!filter_types.empty()) {
        std::stringstream ss(filter_types);
        std::string type;
        while (std::getline(ss, type, ',')) {
            filter_config.allowed_types.push_back(type);
        }
    }
    
    // 建立下載管理器
    DownloadManager download_manager(filter_config, threads);
    g_download_manager = &download_manager;
    
    std::cout << "\n開始下載...\n" << std::endl;
    
    // 設定進度回呼
    download_manager.set_progress_callback([](const DownloadItem& item) {
        // 進度更新已在download_file中處理
    });
    
    // 開始下載
    download_manager.start_download(url, output_dir);
    
    // 等待完成
    download_manager.wait_for_completion();
    
    // 顯示統計
    download_manager.show_statistics();
    
    std::cout << "\n下載完成！\n" << std::endl;
}

int main(int argc, char* argv[]) {
    // 設定信號處理
    signal(SIGINT, signal_handler);
    
    // 解析命令列參數
    std::string url;
    std::string output_dir = "./download";
    int depth = 0;
    bool external = false;
    int threads = 4;
    std::vector<std::string> filter_types;
    long min_size = 0;
    long max_size = -1;
    bool interactive = false;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-u" || arg == "--url") {
            if (i + 1 < argc) url = argv[++i];
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) output_dir = argv[++i];
        } else if (arg == "-d" || arg == "--depth") {
            if (i + 1 < argc) depth = std::stoi(argv[++i]);
        } else if (arg == "-e" || arg == "--external") {
            external = true;
        } else if (arg == "-t" || arg == "--threads") {
            if (i + 1 < argc) threads = std::stoi(argv[++i]);
        } else if (arg == "-f" || arg == "--filter") {
            if (i + 1 < argc) {
                std::string types = argv[++i];
                std::stringstream ss(types);
                std::string type;
                while (std::getline(ss, type, ',')) {
                    filter_types.push_back(type);
                }
            }
        } else if (arg == "-m" || arg == "--min-size") {
            if (i + 1 < argc) min_size = std::stol(argv[++i]) * 1024;
        } else if (arg == "-x" || arg == "--max-size") {
            if (i + 1 < argc) max_size = std::stol(argv[++i]) * 1024;
        } else if (arg == "-i" || arg == "--interactive") {
            interactive = true;
        } else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        }
    }
    
    // 檢查必要參數
    if (!interactive && url.empty()) {
        std::cerr << "錯誤: 必須指定URL或使用互動模式\n" << std::endl;
        print_usage(argv[0]);
        return 1;
    }
    
    // 設定篩選器
    FilterConfig filter_config;
    filter_config.max_depth = depth;
    filter_config.download_external = external;
    filter_config.min_file_size = min_size;
    filter_config.max_file_size = max_size;
    filter_config.allowed_types = filter_types;
    
    if (!interactive) {
        filter_config.base_host = URLFilter::extract_host(url);
    }
    
    // 執行
    if (interactive) {
        interactive_mode();
    } else {
        DownloadManager download_manager(filter_config, threads);
        g_download_manager = &download_manager;
        
        std::cout << "開始下載: " << url << std::endl;
        std::cout << "輸出目錄: " << output_dir << std::endl;
        std::cout << "遞迴深度: " << depth << std::endl;
        std::cout << "線程數量: " << threads << std::endl;
        std::cout << "外部網站: " << (external ? "是" : "否") << std::endl;
        if (!filter_types.empty()) {
            std::cout << "檔案類型: ";
            for (const auto& type : filter_types) {
                std::cout << type << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "==============================" << std::endl;
        
        // 設定進度回呼
        download_manager.set_progress_callback([](const DownloadItem& item) {
            // 進度更新已在download_file中處理
        });
        
        // 開始下載
        download_manager.start_download(url, output_dir);
        
        // 等待完成
        download_manager.wait_for_completion();
        
        // 顯示統計
        download_manager.show_statistics();
        
        std::cout << "\n下載完成！\n" << std::endl;
    }
    
    return 0;
}
