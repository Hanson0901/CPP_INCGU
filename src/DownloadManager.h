// DownloadManager.h
#ifndef DOWNLOAD_MANAGER_H
#define DOWNLOAD_MANAGER_H

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include "HTTPClient.h"
#include "FileManager.h"

class DownloadManager {
private:
    int maxConcurrentDownloads;
    int maxDownloadDepth;
    bool downloadExternalContent;
    std::vector<std::string> allowedFileTypes;  // 空=全部
    long minFileSize;
    long maxFileSize;
    
    std::queue<std::string> downloadQueue;
    std::vector<std::thread> workerThreads;
    std::mutex queueMutex;
    std::atomic<bool> isRunning;
    std::atomic<bool> isPaused;
    
    HTTPClient httpClient;
    FileManager fileManager;
    
    void workerThread();
    bool shouldDownload(const std::string& url, long fileSize);
    void recursiveDownload(const std::string& url, int currentDepth);
    
public:
    DownloadManager(const std::string& outputDir, int maxThreads);
    
    void setDownloadDepth(int depth);
    void setExternalContentPolicy(bool allow);
    void setFileTypeFilter(const std::vector<std::string>& types);
    void setFileSizeRange(long min, long max);
    
    // 開始下載
    void start(const std::string& startURL);
    
    // 暫停/繼續
    void pause();
    void resume();
    
    // 停止並選擇動作
    void stop(bool deleteDownloaded = false);
    
    // 獲取進度
    std::vector<DownloadFile> getCurrentProgress() const;
    FileManager::Statistics getStatistics() const;
    
    ~DownloadManager();
};

#endif
