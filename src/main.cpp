// main.cpp
#include <iostream>
#include <string>
#include "UIManager.h"
#include "DownloadManager.h"

int main(int argc, char* argv[]) {
    std::string url, outputDir;
    int depth = 2;
    bool downloadExternal = false;
    
    // 判定使用命令列或交互模式
    if (argc > 1) {
        // 命令列模式
        if (!UIManager::parseCommandLine(argc, argv, url, outputDir, 
                                        depth, downloadExternal)) {
            std::cerr << "命令列參數錯誤" << std::endl;
            return 1;
        }
    } else {
        // 交互模式
        UIManager::interactiveMode();
        return 0;
    }
    
    // 初始化下載管理器
    DownloadManager manager(outputDir, 4);  // 最多4個同時下載
    manager.setDownloadDepth(depth);
    manager.setExternalContentPolicy(downloadExternal);
    
    // 設置進階選項（可選）
    std::vector<std::string> fileTypes = {"jpg", "png", "html", "css"};
    manager.setFileTypeFilter(fileTypes);
    manager.setFileSizeRange(0, 50 * 1024 * 1024);  // 50MB上限
    
    // 開始下載
    manager.start(url);
    
    // 等待下載完成或用戶中斷
    // （需要實現交互式控制）
    
    return 0;
}
