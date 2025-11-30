// UIManager.h
#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <string>
#include <vector>

class UIManager {
public:
    // 交互模式
    static void interactiveMode();
    
    // 命令列模式
    static bool parseCommandLine(int argc, char* argv[],
                                 std::string& url,
                                 std::string& outputDir,
                                 int& depth,
                                 bool& downloadExternal);
    
    // 顯示進度
    static void displayProgress(const std::vector<DownloadFile>& files,
                               const FileManager::Statistics& stats);
    
    // 主菜單
    static void showMainMenu();
    
private:
    static std::string readInput(const std::string& prompt);
    static void clearScreen();
};

#endif
