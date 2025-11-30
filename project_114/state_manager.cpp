#include "state_manager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

StateManager::StateManager(const std::string& output_dir) {
    state_file = output_dir + "/.download_state.txt";
}

void StateManager::add_item(const DownloadItem& item) {
    std::lock_guard<std::mutex> lock(mutex);
    items.push_back(item);
}

void StateManager::update_item(const std::string& url, DownloadStatus status,
                              long downloaded_size, double speed) {
    std::lock_guard<std::mutex> lock(mutex);
    for (auto& item : items) {
        if (item.url == url) {
            item.status = status;
            if (downloaded_size > 0) {
                item.downloaded_size = downloaded_size;
            }
            if (speed > 0) {
                item.speed = speed;
            }
            if (status == DownloadStatus::DOWNLOADING && item.start_time.time_since_epoch().count() == 0) {
                item.start_time = std::chrono::steady_clock::now();
            }
            if (status == DownloadStatus::COMPLETED || status == DownloadStatus::FAILED) {
                item.end_time = std::chrono::steady_clock::now();
            }
            break;
        }
    }
}

DownloadItem* StateManager::get_item(const std::string& url) {
    std::lock_guard<std::mutex> lock(mutex);
    for (auto& item : items) {
        if (item.url == url) {
            return &item;
        }
    }
    return nullptr;
}

bool StateManager::load_state() {
    std::lock_guard<std::mutex> lock(mutex);
    std::ifstream file(state_file);
    if (!file.is_open()) {
        return false;
    }
    
    items.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::istringstream iss(line);
        DownloadItem item;
        int status_int;
        
        iss >> item.url >> item.local_path >> status_int >> item.total_size 
            >> item.downloaded_size >> item.speed >> item.error_code;
        
        item.status = static_cast<DownloadStatus>(status_int);
        
        if (item.status == DownloadStatus::COMPLETED || item.status == DownloadStatus::FAILED) {
            // 檢查檔案是否存在
            std::ifstream test_file(item.local_path);
            if (!test_file.good()) {
                item.status = DownloadStatus::PENDING;
                item.downloaded_size = 0;
            }
        }
        
        items.push_back(item);
    }
    
    file.close();
    return true;
}

bool StateManager::save_state() {
    std::lock_guard<std::mutex> lock(mutex);
    std::ofstream file(state_file);
    if (!file.is_open()) {
        return false;
    }
    
    for (const auto& item : items) {
        file << item.url << " " << item.local_path << " " 
             << static_cast<int>(item.status) << " " << item.total_size << " "
             << item.downloaded_size << " " << std::fixed << std::setprecision(2) 
             << item.speed << " " << item.error_code << std::endl;
    }
    
    file.close();
    return true;
}

std::vector<DownloadItem> StateManager::get_all_items() const {
    std::lock_guard<std::mutex> lock(mutex);
    return items;
}

void StateManager::clear() {
    std::lock_guard<std::mutex> lock(mutex);
    items.clear();
}

void StateManager::get_statistics(int& total_files, int& completed_files, int& failed_files,
                                 long& total_size, long& downloaded_size, double& total_time) {
    std::lock_guard<std::mutex> lock(mutex);
    total_files = items.size();
    completed_files = 0;
    failed_files = 0;
    total_size = 0;
    downloaded_size = 0;
    total_time = 0.0;
    
    auto now = std::chrono::steady_clock::now();
    for (const auto& item : items) {
        if (item.status == DownloadStatus::COMPLETED) {
            completed_files++;
            total_size += item.total_size;
            downloaded_size += item.total_size;
            
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                item.end_time - item.start_time).count();
            total_time += duration;
        } else if (item.status == DownloadStatus::FAILED) {
            failed_files++;
        } else if (item.status == DownloadStatus::DOWNLOADING) {
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(
                now - item.start_time).count();
            total_time += duration;
            downloaded_size += item.downloaded_size;
        }
    }
}
