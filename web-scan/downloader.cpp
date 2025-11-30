

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <cstring>
#include <regex>
#include <algorithm>
#include <set>
#include <cstdlib>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <conio.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
    #define CLOSE_SOCKET(s) closesocket(s)
    #define SLEEP(ms) Sleep(ms)
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <dirent.h>
    #include <sys/stat.h>
    #define CLOSE_SOCKET(s) close(s)
    #define SLEEP(ms) usleep((ms)*1000)
#endif

std::string sanitizeFilename(const std::string& filename) {
    std::string result = filename;
    const char illegal[] = "<>:\"/\\|?*";
    for (size_t i = 0; i < result.length(); ++i) {
        for (const char* p = illegal; *p; ++p) {
            if (result[i] == *p) {
                result[i] = '_';
                break;
            }
        }
    }
    return result;
}

bool createDirectory(const std::string& path) {
#ifdef _WIN32
    return CreateDirectoryA(path.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
#else
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

bool createDirectoryRecursive(const std::string& path) {
    if (path.empty()) return true;
    if (createDirectory(path)) return true;
    
    size_t lastSlash = path.rfind('/');
    if (lastSlash == std::string::npos) {
        lastSlash = path.rfind('\\');
    }
    
    if (lastSlash == std::string::npos) {
        return false;
    }
    
    std::string parentPath = path.substr(0, lastSlash);
    if (!createDirectoryRecursive(parentPath)) {
        return false;
    }
    
    return createDirectory(path);
}

void removeDirRecursive(const std::string& path) {
#ifdef _WIN32
    std::string cmd = "rmdir /s /q \"" + path + "\" >nul 2>&1";
    int result = system(cmd.c_str());
    SLEEP(500);
#else
    std::string cmd = "rm -rf \"" + path + "\"";
    int result = system(cmd.c_str());
    SLEEP(500);
#endif
}

bool fileExists(const std::string& path) {
#ifdef _WIN32
    return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
#else
    return access(path.c_str(), F_OK) != -1;
#endif
}

void clearInputBuffer() {
#ifdef _WIN32
    while (_kbhit()) _getch();
#else
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
#endif
}

bool kbhit_custom() {
#ifdef _WIN32
    return _kbhit() != 0;
#else
    return false;
#endif
}

char getch_custom() {
#ifdef _WIN32
    return _getch();
#else
    return 'q';
#endif
}

class SocketInitializer {
public:
    SocketInitializer() {
#ifdef _WIN32
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
            std::cerr << "WSAStartup 失敗" << std::endl;
        }
#endif
    }
    
    ~SocketInitializer() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
};

static SocketInitializer socketInit;

struct URL {
    std::string protocol;
    std::string host;
    int port;
    std::string path;
    std::string query;
    
    std::string getRequestPath() const {
        std::string result = path.empty() ? "/" : path;
        if (!query.empty()) result += "?" + query;
        return result;
    }
};

class URLParser {
public:
    static URL parse(const std::string& urlStr) {
        URL url;
        url.port = 80;
        url.protocol = "http";
        
        std::string remaining = urlStr;
        
        size_t protocolEnd = remaining.find("://");
        if (protocolEnd != std::string::npos) {
            url.protocol = remaining.substr(0, protocolEnd);
            remaining = remaining.substr(protocolEnd + 3);
            if (url.protocol == "https") url.port = 443;
        }
        
        size_t pathStart = remaining.find('/');
        std::string hostPart = (pathStart == std::string::npos) ? remaining : remaining.substr(0, pathStart);
        
        size_t portStart = hostPart.find(':');
        if (portStart != std::string::npos) {
            url.host = hostPart.substr(0, portStart);
            url.port = std::stoi(hostPart.substr(portStart + 1));
        } else {
            url.host = hostPart;
        }
        
        if (pathStart != std::string::npos) {
            remaining = remaining.substr(pathStart);
            size_t queryStart = remaining.find('?');
            if (queryStart != std::string::npos) {
                url.path = remaining.substr(0, queryStart);
                url.query = remaining.substr(queryStart + 1);
            } else {
                url.path = remaining;
            }
        } else {
            url.path = "/";
        }
        
        return url;
    }
    
    static std::string resolveRelativeURL(const std::string& baseURL, const std::string& relativeURL) {
        if (relativeURL.find("://") != std::string::npos) {
            return relativeURL;
        }
        
        URL baseUrl = parse(baseURL);
        
        if (relativeURL[0] == '/') {
            return baseUrl.protocol + "://" + baseUrl.host +
                   (baseUrl.port != 80 && baseUrl.port != 443 ? ":" + std::to_string(baseUrl.port) : "") +
                   relativeURL;
        }
        
        size_t lastSlash = baseUrl.path.rfind('/');
        std::string basePath = (lastSlash != std::string::npos) ? baseUrl.path.substr(0, lastSlash + 1) : "/";
        
        return baseUrl.protocol + "://" + baseUrl.host +
               (baseUrl.port != 80 && baseUrl.port != 443 ? ":" + std::to_string(baseUrl.port) : "") +
               basePath + relativeURL;
    }
    
    static bool isExternalURL(const std::string& baseHost, const std::string& checkURL) {
        URL url = parse(checkURL);
        return url.host != baseHost;
    }
};

struct HTTPResponse {
    int statusCode;
    std::map<std::string, std::string> headers;
    std::vector<char> body;
    
    HTTPResponse() : statusCode(0) {}
};

class HTTPClient {
private:
    static const int BUFFER_SIZE = 8192;
    
    int createSocket(const std::string& host, int port) {
        struct hostent* server = gethostbyname(host.c_str());
        if (!server) {
            return -1;
        }

        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            return -1;
        }

        struct sockaddr_in serv_addr;
        std::memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        std::memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

        if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            CLOSE_SOCKET(sockfd);
            return -1;
        }

        return sockfd;
    }
    
    bool sendRequest(int sockfd, const std::string& method, const std::string& path,
                    const std::string& host, int port) {
        std::string request = method + " " + path + " HTTP/1.1\r\n";
        request += "Host: " + host + (port != 80 ? ":" + std::to_string(port) : "") + "\r\n";
        request += "Connection: close\r\n";
        request += "User-Agent: WebDownloader/1.0\r\n";
        request += "Accept: */*\r\n";
        request += "\r\n";

        return send(sockfd, request.c_str(), (int)request.length(), 0) > 0;
    }
    
    HTTPResponse receiveResponse(int sockfd) {
        HTTPResponse response;
        std::vector<char> buffer;
        char chunk[BUFFER_SIZE];
        int bytes;

        while ((bytes = recv(sockfd, chunk, BUFFER_SIZE - 1, 0)) > 0) {
            buffer.insert(buffer.end(), chunk, chunk + bytes);
        }

        CLOSE_SOCKET(sockfd);

        std::string rawResponse(buffer.begin(), buffer.end());
        
        size_t headerEnd = rawResponse.find("\r\n\r\n");
        if (headerEnd == std::string::npos) {
            return response;
        }

        std::string headerStr = rawResponse.substr(0, headerEnd);
        std::string bodyStr = rawResponse.substr(headerEnd + 4);

        size_t firstCR = headerStr.find("\r\n");
        std::string statusLine = headerStr.substr(0, firstCR);
        
        size_t codeStart = statusLine.find(' ') + 1;
        size_t codeEnd = statusLine.find(' ', codeStart);
        std::string codeStr = statusLine.substr(codeStart, codeEnd - codeStart);
        response.statusCode = std::stoi(codeStr);

        response.body = std::vector<char>(bodyStr.begin(), bodyStr.end());

        return response;
    }
    
public:
    HTTPResponse GET(const std::string& url) {
        URL parsedUrl = URLParser::parse(url);
        
        int sockfd = createSocket(parsedUrl.host, parsedUrl.port);
        if (sockfd < 0) {
            HTTPResponse errorResp;
            errorResp.statusCode = 0;
            return errorResp;
        }

        sendRequest(sockfd, "GET", parsedUrl.getRequestPath(), parsedUrl.host, parsedUrl.port);
        return receiveResponse(sockfd);
    }
};

class HTMLParser {
public:
    static std::vector<std::string> extractLinks(const std::string& html) {
        std::vector<std::string> links;
        
        std::regex hrefRegex(R"(href\s*=\s*[""']?([^""'\s>]+)[""']?)", 
                            std::regex::icase);
        std::smatch match;
        
        std::string::const_iterator searchStart(html.cbegin());
        while (std::regex_search(searchStart, html.cend(), match, hrefRegex)) {
            links.push_back(match[1]);
            searchStart = match.suffix().first;
        }
        
        return links;
    }
    
    static std::vector<std::string> extractImages(const std::string& html) {
        std::vector<std::string> images;
        
        std::regex srcRegex(R"(src\s*=\s*[""']?([^""'\s>]+)[""']?)", 
                           std::regex::icase);
        std::smatch match;
        
        std::string::const_iterator searchStart(html.cbegin());
        while (std::regex_search(searchStart, html.cend(), match, srcRegex)) {
            images.push_back(match[1]);
            searchStart = match.suffix().first;
        }
        
        return images;
    }
};

class DownloadManager {
private:
    int maxDepth;
    bool downloadExternal;
    std::string basePath;
    std::string baseHost;
    std::string startURL;
    std::string initialURL;
    std::set<std::string> processedURLs;
    std::set<std::string> failedURLs;
    std::vector<std::pair<std::string, int> > downloadQueue;
    HTTPClient httpClient;
    int totalFiles;
    int completedFiles;
    int failedFiles;
    int state;
    
    void saveState() {
        std::string stateFile = basePath + "/.download_state";
        std::ofstream file(stateFile.c_str());
        if (!file) {
            return;
        }
        
        for (std::set<std::string>::iterator it = processedURLs.begin(); it != processedURLs.end(); ++it) {
            file << "PROCESSED:" << *it << std::endl;
        }
        
        for (size_t i = 0; i < downloadQueue.size(); ++i) {
            file << "QUEUE:" << downloadQueue[i].first << ":" << downloadQueue[i].second << std::endl;
        }
        
        file.close();
    }
    
    void loadState() {
        std::string stateFile = basePath + "/.download_state";
        std::ifstream file(stateFile.c_str());
        if (!file) {
            downloadQueue.push_back(std::make_pair(startURL, 0));
            return;
        }
        
        downloadQueue.clear();
        std::string line;
        
        while (std::getline(file, line)) {
            if (line.substr(0, 10) == "PROCESSED:") {
                std::string url = line.substr(10);
                if (!url.empty()) {
                    processedURLs.insert(url);
                }
            } else if (line.substr(0, 6) == "QUEUE:") {
                size_t lastColon = line.rfind(':');
                if (lastColon != std::string::npos && lastColon > 6) {
                    std::string url = line.substr(6, lastColon - 6);
                    std::string depthStr = line.substr(lastColon + 1);
                    
                    if (!url.empty() && !depthStr.empty()) {
                        int depth = std::stoi(depthStr);
                        downloadQueue.push_back(std::make_pair(url, depth));
                    }
                }
            }
        }
        
        file.close();
    }
    
    void deleteState() {
        std::string stateFile = basePath + "/.download_state";
        if (fileExists(stateFile)) {
            remove(stateFile.c_str());
        }
    }
    
    void printMenu() {
        std::cout << "\n===== 【暫停中】 =====" << std::endl;
        std::cout << " [C] 繼續下載" << std::endl;
        std::cout << " [R] 重新開始" << std::endl;
        std::cout << " [S] 統計資訊" << std::endl;
        std::cout << " [Q] 退出" << std::endl;
        std::cout << "====================\n" << std::endl;
    }
    
    int handlePause() {
        clearInputBuffer();
        printMenu();
        
        bool waiting = true;
        while (waiting) {
            if (kbhit_custom()) {
                char ch = getch_custom();
                ch = tolower((unsigned char)ch);
                
                switch (ch) {
                    case 'c':
                        std::cout << "繼續下載...\n" << std::endl;
                        clearInputBuffer();
                        return 0;
                        
                    case 'r':
                        std::cout << "重新開始...\n" << std::endl;
                        clearInputBuffer();
                        return 1;
                        
                    case 's':
                        printStatistics();
                        printMenu();
                        break;
                        
                    case 'q':
                        std::cout << "退出...\n" << std::endl;
                        clearInputBuffer();
                        return 2;
                }
            }
            
            SLEEP(100);
        }
        
        return 0;
    }
    
    void resetAllState() {
        processedURLs.clear();
        failedURLs.clear();
        downloadQueue.clear();
        totalFiles = 0;
        completedFiles = 0;
        failedFiles = 0;
        startURL = initialURL;
        baseHost = URLParser::parse(initialURL).host;
    }
    
public:
    DownloadManager(const std::string& outputDir) 
        : basePath(outputDir), maxDepth(2), downloadExternal(false), 
          totalFiles(0), completedFiles(0), failedFiles(0), state(0) {
        createDirectoryRecursive(basePath);
    }
    
    void setDepth(int depth) { maxDepth = depth; }
    void setDownloadExternal(bool external) { downloadExternal = external; }
    
    std::string generateLocalPath(const std::string& url) {
        URL parsedUrl = URLParser::parse(url);
        
        std::string hostDir = basePath + "/" + sanitizeFilename(parsedUrl.host);
        createDirectoryRecursive(hostDir);
        
        std::string filepath = hostDir + parsedUrl.path;
        
        for (size_t i = 0; i < filepath.length(); ++i) {
            if (filepath[i] == '?') {
                filepath = filepath.substr(0, i);
                break;
            }
        }
        
        std::string cleanedPath;
        std::string segment;
        for (size_t i = 0; i < filepath.length(); ++i) {
            if (filepath[i] == '/' || filepath[i] == '\\') {
                if (!segment.empty()) {
                    segment = sanitizeFilename(segment);
                    cleanedPath += segment + "/";
                    segment.clear();
                } else {
                    cleanedPath += "/";
                }
            } else {
                segment += filepath[i];
            }
        }
        if (!segment.empty()) {
            segment = sanitizeFilename(segment);
            cleanedPath += segment;
        }
        
        filepath = cleanedPath;
        
        if (!filepath.empty() && (filepath.back() == '/' || filepath.back() == '\\')) {
            filepath += "index.html";
        }
        
        size_t lastSlash = filepath.rfind('/');
        if (lastSlash == std::string::npos) {
            lastSlash = filepath.rfind('\\');
        }
        
        if (lastSlash != std::string::npos) {
            createDirectoryRecursive(filepath.substr(0, lastSlash));
        }
        
        return filepath;
    }
    
    bool saveFile(const std::string& url, const std::vector<char>& data) {
        std::string localPath = generateLocalPath(url);
        
        if (localPath.empty()) {
            return false;
        }
        
        std::ofstream file(localPath.c_str(), std::ios::binary);
        if (!file) {
            return false;
        }
        
        file.write(data.data(), data.size());
        file.close();
        return true;
    }
    
    void start(const std::string& url) {
        initialURL = url;
        startURL = url;
        baseHost = URLParser::parse(url).host;
        
        std::cout << "\n開始下載..." << std::endl;
        std::cout << "提示：按 'P' 暫停\n" << std::endl;
        
        downloadQueue.push_back(std::make_pair(url, 0));
        
        // 主迴圈
        while (!downloadQueue.empty()) {
            // 檢查 P 鍵
            if (kbhit_custom()) {
                char ch = getch_custom();
                ch = tolower((unsigned char)ch);
                if (ch == 'p') {
                    saveState();
                    int pauseResult = handlePause();
                    
                    if (pauseResult == 2) {
                        deleteState();
                        return;
                    } else if (pauseResult == 1) {
                        // 重新開始
                        std::cout << "刪除已下載檔案...\n" << std::endl;
                        if (fileExists(basePath)) {
                            removeDirRecursive(basePath);
                            SLEEP(1000);
                        }
                        createDirectoryRecursive(basePath);
                        deleteState();
                        resetAllState();
                        SLEEP(200);
                        clearInputBuffer();
                        downloadQueue.push_back(std::make_pair(initialURL, 0));
                        continue;  // ✨ 回到主迴圈，繼續下載
                    } else {
                        // 繼續下載
                        SLEEP(200);
                        clearInputBuffer();
                        loadState();
                        continue;
                    }
                }
            }
            
            if (downloadQueue.empty()) {
                break;
            }
            
            std::pair<std::string, int> task = downloadQueue.front();
            downloadQueue.erase(downloadQueue.begin());
            
            std::string urlStr = task.first;
            int depth = task.second;
            
            if (processedURLs.find(urlStr) != processedURLs.end()) {
                continue;
            }
            processedURLs.insert(urlStr);
            
            std::cout << "[下載] " << urlStr << std::endl;
            totalFiles++;
            
            HTTPResponse response = httpClient.GET(urlStr);
            if (response.statusCode != 200) {
                failedFiles++;
                failedURLs.insert(urlStr);
                continue;
            }
            
            if (!saveFile(urlStr, response.body)) {
                failedFiles++;
                failedURLs.insert(urlStr);
                continue;
            }
            
            completedFiles++;
            
            if (depth < maxDepth) {
                std::string htmlContent(response.body.begin(), response.body.end());
                
                std::vector<std::string> links = HTMLParser::extractLinks(htmlContent);
                std::vector<std::string> images = HTMLParser::extractImages(htmlContent);
                
                for (size_t i = 0; i < links.size(); ++i) {
                    if (!links[i].empty() && links[i][0] != '#') {
                        std::string resolvedURL = URLParser::resolveRelativeURL(urlStr, links[i]);
                        
                        if (!downloadExternal && URLParser::isExternalURL(baseHost, resolvedURL)) {
                            continue;
                        }
                        
                        if (processedURLs.find(resolvedURL) == processedURLs.end()) {
                            downloadQueue.push_back(std::make_pair(resolvedURL, depth + 1));
                        }
                    }
                }
                
                for (size_t i = 0; i < images.size(); ++i) {
                    if (!images[i].empty()) {
                        std::string resolvedURL = URLParser::resolveRelativeURL(urlStr, images[i]);
                        
                        if (!downloadExternal && URLParser::isExternalURL(baseHost, resolvedURL)) {
                            continue;
                        }
                        
                        if (processedURLs.find(resolvedURL) == processedURLs.end()) {
                            downloadQueue.push_back(std::make_pair(resolvedURL, depth + 1));
                        }
                    }
                }
            }
            
            if (totalFiles % 3 == 0) {
                saveState();
            }
        }
        
        deleteState();
        printStatistics();
        std::cout << "下載完成！\n" << std::endl;
    }
    
    void printStatistics() {
        std::cout << "\n====== 統計資訊 ======" << std::endl;
        std::cout << "總檔案數: " << totalFiles << std::endl;
        std::cout << "已完成: " << completedFiles << std::endl;
        std::cout << "失敗: " << failedFiles << std::endl;
        std::cout << "等待中: " << downloadQueue.size() << std::endl;
        if (totalFiles > 0) {
            std::cout << "進度: " << (completedFiles * 100 / totalFiles) << "%" << std::endl;
        }
        std::cout << "===================\n" << std::endl;
    }
};

void printUsage() {
    std::cout << "用法:" << std::endl;
    std::cout << "  downloader <URL> <目錄> [深度] [外部內容]" << std::endl;
    std::cout << "  例: downloader http://example.com ./output 2 0" << std::endl;
}

void interactiveMode() {
    std::cout << "=== 網站離線下載器 ===\n" << std::endl;
    
    std::string url, outputDir, depthStr, externalStr;
    int depth, external;
    
    std::cout << "URL: ";
    std::getline(std::cin, url);
    
    std::cout << "輸出目錄 (預設 ./download): ";
    std::getline(std::cin, outputDir);
    if (outputDir.empty()) outputDir = "./download";
    
    std::cout << "遞迴深度 (預設 2): ";
    std::getline(std::cin, depthStr);
    depth = depthStr.empty() ? 2 : std::stoi(depthStr);
    
    std::cout << "下載外部內容? (0=否 1=是, 預設 0): ";
    std::getline(std::cin, externalStr);
    external = externalStr.empty() ? 0 : std::stoi(externalStr);
    
    DownloadManager manager(outputDir);
    manager.setDepth(depth);
    manager.setDownloadExternal(external == 1);
    manager.start(url);
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        interactiveMode();
    } else if (argc >= 3) {
        std::string url = argv[1];
        std::string outputDir = argv[2];
        int depth = (argc >= 4) ? std::stoi(argv[3]) : 2;
        int external = (argc >= 5) ? std::stoi(argv[4]) : 0;
        
        DownloadManager manager(outputDir);
        manager.setDepth(depth);
        manager.setDownloadExternal(external == 1);
        manager.start(url);
    } else {
        printUsage();
        return 1;
    }
    
    return 0;
}