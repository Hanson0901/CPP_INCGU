#ifndef DOWNLOAD_STATE_H
#define DOWNLOAD_STATE_H

#include <string>
#include <chrono>

enum class DownloadStatus {
    PENDING,
    DOWNLOADING,
    COMPLETED,
    FAILED,
    SKIPPED
};

struct DownloadItem {
    std::string url;
    std::string local_path;
    DownloadStatus status;
    long total_size;
    long downloaded_size;
    double speed;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    int error_code;
    
    DownloadItem() : status(DownloadStatus::PENDING), total_size(0), 
                    downloaded_size(0), speed(0.0), error_code(0) {}
};

#endif
