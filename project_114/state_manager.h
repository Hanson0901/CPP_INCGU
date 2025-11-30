#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include "download_state.h"
#include <vector>
#include <string>
#include <mutex>

class StateManager {
private:
    std::string state_file;
    std::vector<DownloadItem> items;
    std::mutex mutex;
    
public:
    StateManager(const std::string& output_dir);
    
    // 添加下載項目
    void add_item(const DownloadItem& item);
    
    // 更新下載項目狀態
    void update_item(const std::string& url, DownloadStatus status, 
                     long downloaded_size = 0, double speed = 0.0);
    
    // 獲取下載項目
    DownloadItem* get_item(const std::string& url);
    
    // 載入狀態
    bool load_state();
    
    // 儲存狀態
    bool save_state();
    
    // 獲取所有項目
    std::vector<DownloadItem> get_all_items() const;
    
    // 清除所有項目
    void clear();
    
    // 獲取統計資訊
    void get_statistics(int& total_files, int& completed_files, int& failed_files,
                       long& total_size, long& downloaded_size, double& total_time);
};

#endif
