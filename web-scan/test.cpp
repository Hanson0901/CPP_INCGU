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
#include <cctype>

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
    #include <errno.h>
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

bool fileExists(const std::string& path) {
#ifdef _WIN32
    return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
#else
    return access(path.c_str(), F_OK) != -1;
#endif
}

void removeDirRecursive(const std::string& path) {
    if (!fileExists(path)) {
        return;
    }
    
    std::cerr << "[DEBUG] 正在删除目录: " << path << std::endl;
    
#ifdef _WIN32
    std::string cmd = "rmdir /s /q \"" + path + "\" 2>nul";
    int result = system(cmd.c_str());
    
    for (int i = 0; i < 20; ++i) {
        if (!fileExists(path)) {
            std::cerr << "[DEBUG] 目录已删除 (第 " << (i+1) << " 次检查成功)" << std::endl;
            return;
        }
        std::cerr << "[DEBUG] 等待目录删除... (" << (i+1) << "/20)" << std::endl;
        SLEEP(1000);
    }
    
    if (fileExists(path)) {
        std::cerr << "[WARNING] rmdir 仍未完成,尝试 PowerShell 删除..." << std::endl;
        cmd = "powershell -Command \"Remove-Item -Path '" + path + "' -Recurse -Force -ErrorAction SilentlyContinue\"";
        system(cmd.c_str());
        SLEEP(2000);
    }
#else
    std::string cmd = "rm -rf \"" + path + "\"";
    int result = system(cmd.c_str());
    
    for (int i = 0; i < 10; ++i) {
        if (!fileExists(path)) {
            std::cerr << "[DEBUG] 目录已删除 (第 " << (i+1) << " 次检查成功)" << std::endl;
            return;
        }
        std::cerr << "[DEBUG] 等待目录删除... (" << (i+1) << "/10)" << std::endl;
        SLEEP(500);
    }
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
            std::cerr << "WSAStartup 失败" << std::endl;
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
    
    static std::string normalizePath(const std::string& path) {
        std::vector<std::string> segments;
        std::string current;
        
        for (size_t i = 0; i < path.length(); ++i) {
            if (path[i] == '/') {
                if (!current.empty()) {
                    if (current == "..") {
                        if (!segments.empty()) {
                            segments.pop_back();
                        }
                    } else if (current != ".") {
                        segments.push_back(current);
                    }
                    current.clear();
                }
            } else {
                current += path[i];
            }
        }
        
        if (!current.empty()) {
            if (current == "..") {
                if (!segments.empty()) {
                    segments.pop_back();
                }
            } else if (current != ".") {
                segments.push_back(current);
            }
        }
        
        std::string result = "/";
        for (size_t i = 0; i < segments.size(); ++i) {
            result += segments[i];
            if (i < segments.size() - 1) {
                result += "/";
            }
        }
        
        return result;
    }
    
    static std::string resolveRelativeURL(const std::string& baseURL, const std::string& relativeURL) {
        if (relativeURL.empty()) {
            return baseURL;
        }
        
        if (relativeURL.find("://") != std::string::npos) {
            return relativeURL;
        }
        
        if (relativeURL.length() >= 2 && relativeURL[0] == '/' && relativeURL[1] == '/') {
            URL baseUrl = parse(baseURL);
            return baseUrl.protocol + ":" + relativeURL;
        }
        
        URL baseUrl = parse(baseURL);
        
        if (relativeURL[0] == '/') {
            std::string result = baseUrl.protocol + "://" + baseUrl.host;
            if ((baseUrl.port != 80 && baseUrl.port != 443) || 
                (baseUrl.protocol == "https" && baseUrl.port != 443) ||
                (baseUrl.protocol == "http" && baseUrl.port != 80)) {
                result += ":" + std::to_string(baseUrl.port);
            }
            result += normalizePath(relativeURL);
            return result;
        }
        
        size_t lastSlash = baseUrl.path.rfind('/');
        std::string basePath;
        
        if (lastSlash != std::string::npos) {
            basePath = baseUrl.path.substr(0, lastSlash + 1);
        } else {
            basePath = "/";
        }
        
        std::string combinedPath = basePath + relativeURL;
        combinedPath = normalizePath(combinedPath);
        
        std::string result = baseUrl.protocol + "://" + baseUrl.host;
        if ((baseUrl.port != 80 && baseUrl.port != 443) ||
            (baseUrl.protocol == "https" && baseUrl.port != 443) ||
            (baseUrl.protocol == "http" && baseUrl.port != 80)) {
            result += ":" + std::to_string(baseUrl.port);
        }
        result += combinedPath;
        
        return result;
    }
    
    static bool isValidURL(const std::string& url) {
        if (url.empty()) return false;
        
        if (url.find("://") == std::string::npos) {
            return url != "/" && url != "#";
        }
        
        size_t protocolEnd = url.find("://");
        size_t hostEnd = url.find('/', protocolEnd + 3);
        if (hostEnd == std::string::npos) hostEnd = url.length();
        
        std::string hostPart = url.substr(protocolEnd + 3, hostEnd - protocolEnd - 3);
        return !hostPart.empty();
    }
    
    static bool isExternalURL(const std::string& baseHost, const std::string& checkURL) {
        if (!isValidURL(checkURL)) {
            return true;
        }
        
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
private:
    static std::vector<std::string> extractAttribute(const std::string& html, 
                                                     const std::string& attribute) {
        std::vector<std::string> results;
        
        std::vector<std::string> patterns;
        patterns.push_back(attribute + "=\"([^\"]*)\"");
        patterns.push_back(attribute + "='([^']*)'");
        patterns.push_back(attribute + "=([^\\s>]+)");
        
        try {
            for (size_t p = 0; p < patterns.size(); ++p) {
                std::regex regex(patterns[p], std::regex::icase);
                std::smatch match;
                
                std::string::const_iterator searchStart(html.cbegin());
                while (std::regex_search(searchStart, html.cend(), match, regex)) {
                    std::string value = match[1].str();
                    
                    if (!value.empty()) {
                        std::string cleaned;
                        for (size_t j = 0; j < value.length(); ++j) {
                            if (value[j] != '"' && value[j] != '\'') {
                                cleaned += value[j];
                            }
                        }
                        if (!cleaned.empty()) {
                            results.push_back(cleaned);
                        }
                    }
                    
                    searchStart = match.suffix().first;
                }
            }
        } catch (const std::regex_error& e) {
            std::cerr << "[ERROR] 正则: " << e.what() << std::endl;
        }
        
        return results;
    }
    
    static std::string cleanURL(const std::string& url) {
        if (url.empty()) return "";
        
        std::string cleaned;
        for (char c : url) {
            if (!std::isspace((unsigned char)c)) {
                cleaned += c;
            }
        }
        
        while (!cleaned.empty() && 
               (cleaned.back() == '>' || cleaned.back() == '"')) {
            cleaned.pop_back();
        }
        
        return cleaned;
    }

public:
    static std::vector<std::string> extractLinks(const std::string& html) {
        std::vector<std::string> links;
        std::vector<std::string> hrefValues = extractAttribute(html, "href");
        
        for (const auto& href : hrefValues) {
            std::string link = cleanURL(href);
            if (!link.empty() && link[0] != '#') {
                links.push_back(link);
            }
        }
        
        return links;
    }
    
    static std::vector<std::string> extractImages(const std::string& html) {
        std::vector<std::string> images;
        std::vector<std::string> srcValues = extractAttribute(html, "src");
        
        for (const auto& src : srcValues) {
            std::string img = cleanURL(src);
            if (!img.empty() && img.find("data:") != 0) {
                images.push_back(img);
            }
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
    std::set<std::string> queuedURLs;
    std::vector<std::pair<std::string, int> > downloadQueue;
    HTTPClient httpClient;
    int totalFiles;
    int completedFiles;
    int failedFiles;
    int state;
    
    void saveState() {
        std::string stateFile = basePath + "/download_state";
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
        std::string stateFile = basePath + "/download_state";
        std::ifstream file(stateFile.c_str());
        if (!file) {
            downloadQueue.push_back(std::make_pair(startURL, 0));
            queuedURLs.insert(startURL);
            return;
        }
        
        downloadQueue.clear();
        queuedURLs.clear();
        std::string line;
        
        while (std::getline(file, line)) {
            // 【新增】移除行尾的所有空白字符
            while (!line.empty() && std::isspace((unsigned char)line.back())) {
                line.pop_back();
            }
            
            // 【新增】移除行首的所有空白字符
            size_t start = 0;
            while (start < line.length() && std::isspace((unsigned char)line[start])) {
                start++;
            }
            if (start > 0) {
                line = line.substr(start);
            }
            
            if (line.empty()) {
                continue;
            }
            
            if (line.substr(0, 10) == "PROCESSED:") {
                std::string url = line.substr(10);
                
                // 再次清理 URL
                while (!url.empty() && std::isspace((unsigned char)url.back())) {
                    url.pop_back();
                }
                
                if (!url.empty()) {
                    processedURLs.insert(url);
                }
            } else if (line.substr(0, 6) == "QUEUE:") {
                size_t lastColon = line.rfind(':');
                if (lastColon != std::string::npos && lastColon > 6) {
                    std::string url = line.substr(6, lastColon - 6);
                    std::string depthStr = line.substr(lastColon + 1);
                    
                    // 清理提取的字符串
                    while (!url.empty() && std::isspace((unsigned char)url.back())) {
                        url.pop_back();
                    }
                    while (!depthStr.empty() && std::isspace((unsigned char)depthStr.back())) {
                        depthStr.pop_back();
                    }
                    
                    if (!url.empty() && !depthStr.empty()) {
                        try {
                            int depth = std::stoi(depthStr);
                            downloadQueue.push_back(std::make_pair(url, depth));
                            queuedURLs.insert(url);
                        } catch (...) {
                            std::cerr << "[ERROR] 无法解析深度: " << depthStr << std::endl;
                        }
                    }
                }
            }
        }
        
        file.close();
    }

    
    void deleteState() {
        std::string stateFile = basePath + "/download_state";
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
    
    void safeReset() {
        std::cout << "[DEBUG] 开始清空所有数据..." << std::endl;
        
        processedURLs.clear();
        failedURLs.clear();
        queuedURLs.clear();
        downloadQueue.clear();
        
        totalFiles = 0;
        completedFiles = 0;
        failedFiles = 0;
        
        std::cout << "[DEBUG] 内存数据已清空" << std::endl;
        
        std::string stateFile = basePath + "/download_state";
        if (fileExists(stateFile)) {
            std::cout << "[DEBUG] 删除状态文件..." << std::endl;
            remove(stateFile.c_str());
            SLEEP(200);
        }
        
        if (fileExists(basePath)) {
            std::cout << "[DEBUG] 正在删除下载目录..." << std::endl;
            removeDirRecursive(basePath);
        }
        
        std::cout << "[DEBUG] 正在创建新的下载目录..." << std::endl;
        createDirectoryRecursive(basePath);
        
        startURL = initialURL;
        baseHost = URLParser::parse(initialURL).host;
        
        downloadQueue.push_back(std::make_pair(initialURL, 0));
        
        std::cout << "[DEBUG] safeReset() 完成！" << std::endl;
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
        queuedURLs.insert(url);
        
        while (!downloadQueue.empty()) {
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
                        std::cout << "正在重新開始...\n" << std::endl;
                        safeReset();
                        
                        std::cout << "重新開始完成,繼續下載...\n" << std::endl;
                        SLEEP(500);
                        clearInputBuffer();
                        continue;
                    } else {
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
            
            if (!URLParser::isValidURL(urlStr)) {
                std::cout << "[跳過] 無效 URL: " << urlStr << std::endl;
                continue;
            }
            
            if (processedURLs.find(urlStr) != processedURLs.end()) {
                continue;
            }
            processedURLs.insert(urlStr);
            queuedURLs.erase(urlStr);
            
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
                        
                        if (!URLParser::isValidURL(resolvedURL)) {
                            continue;
                        }
                        
                        if (!downloadExternal && URLParser::isExternalURL(baseHost, resolvedURL)) {
                            continue;
                        }
                        
                        if (processedURLs.find(resolvedURL) == processedURLs.end() &&
                            queuedURLs.find(resolvedURL) == queuedURLs.end()) {
                            downloadQueue.push_back(std::make_pair(resolvedURL, depth + 1));
                            queuedURLs.insert(resolvedURL);
                        }
                    }
                }
                
                for (size_t i = 0; i < images.size(); ++i) {
                    if (!images[i].empty()) {
                        std::string resolvedURL = URLParser::resolveRelativeURL(urlStr, images[i]);
                        
                        if (!URLParser::isValidURL(resolvedURL)) {
                            continue;
                        }
                        
                        if (!downloadExternal && URLParser::isExternalURL(baseHost, resolvedURL)) {
                            continue;
                        }
                        
                        if (processedURLs.find(resolvedURL) == processedURLs.end() &&
                            queuedURLs.find(resolvedURL) == queuedURLs.end()) {
                            downloadQueue.push_back(std::make_pair(resolvedURL, depth + 1));
                            queuedURLs.insert(resolvedURL);
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
