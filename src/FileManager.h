// FileManager.h
#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <ctime>

struct DownloadFile {
    std::string url;
    std::string localPath;
    long fileSize;
    long downloadedSize;
    bool completed;
    std::time_t startTime;
    std::time_t lastUpdateTime;
    
    double getProgress() const {
        if (fileSize == 0) return 0.0;
        return (downloadedSize * 100.0) / fileSize;
    }
    
    double getSpeed() const {  // 字節/秒
        time_t elapsed = lastUpdateTime - startTime;
        if (elapsed == 0) return 0.0;
        return downloadedSize / static_cast<double>(elapsed);
    }
    
    long getETA() const {  // 秒
        double speed = getSpeed();
        if (speed == 0) return -1;
        long remaining = fileSize - downloadedSize;
        return static_cast<long>(remaining / speed);
    }
};

class FileManager {
private:
    std::string basePath;
    std::map<std::string, DownloadFile> downloadMap;
    
public:
    explicit FileManager(const std::string& outputDir);
    
    // 建立輸出目錄
    bool createOutputDirectory();
    
    // 產生本地檔案路徑
    std::string generateLocalPath(const std::string& url);
    
    // 檢查檔案是否已存在且完成
    bool isDownloadedComplete(const std::string& url) const;
    
    // 保存已下載檔案列表（支持斷點續傳）
    void saveDownloadState(const std::string& stateFile);
    void loadDownloadState(const std::string& stateFile);
    
    // 刪除已下載檔案（用於重新下載）
    bool deleteDownloadedFiles();
    
    // 統計信息
    struct Statistics {
        int totalFiles;
        int completedFiles;
        long totalSize;
        long downloadedSize;
        time_t startTime;
    };
    
    Statistics getStatistics() const;
};

#endif
