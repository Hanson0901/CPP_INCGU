// HTTPClient.cpp 核心部分
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

int HTTPClient::createSocket(const std::string& host, int port) {
    // 1. DNS解析
    struct hostent* server = gethostbyname(host.c_str());
    if (!server) {
        std::cerr << "DNS 解析失敗: " << host << std::endl;
        return -1;
    }
    
    // 2. 建立socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return -1;
    
    // 3. 連接到伺服器
    struct sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    std::memcpy(&serv_addr.sin_addr.s_addr, 
                server->h_addr, 
                server->h_length);
    
    if (connect(sockfd, (struct sockaddr*)&serv_addr, 
                sizeof(serv_addr)) < 0) {
        std::cerr << "連接失敗" << std::endl;
        close(sockfd);
        return -1;
    }
    
    return sockfd;
}

bool HTTPClient::sendRequest(int socket, const HTTPRequest& req) {
    // 建立HTTP請求行
    std::string requestLine = req.method + " " + req.url + 
                             " HTTP/1.1\r\n";
    
    // 加入必要的請求頭
    std::string headers = requestLine;
    headers += "Host: " + req.host + "\r\n";
    headers += "Connection: close\r\n";  // 關鍵：關閉持久連接
    headers += "User-Agent: WebDownloader/1.0\r\n";
    
    // 加入自訂請求頭
    for (auto& [key, value] : req.headers) {
        headers += key + ": " + value + "\r\n";
    }
    
    headers += "\r\n";  // 空行表示頭部結束
    
    // 發送請求
    if (send(socket, headers.c_str(), headers.length(), 0) < 0) {
        std::cerr << "發送請求失敗" << std::endl;
        return false;
    }
    
    return true;
}

HTTPResponse HTTPClient::receiveResponse(int socket) {
    HTTPResponse response;
    response.statusCode = 0;
    response.isChunked = false;
    
    std::vector<char> buffer;
    char chunk[BUFFER_SIZE];
    int bytes;
    
    // 接收所有數據
    while ((bytes = recv(socket, chunk, BUFFER_SIZE - 1, 0)) > 0) {
        buffer.insert(buffer.end(), chunk, chunk + bytes);
    }
    
    close(socket);
    
    // 轉換為字符串用於解析
    std::string rawResponse(buffer.begin(), buffer.end());
    
    // 1. 分割頭部和主體（關鍵：\r\n\r\n分隔）
    size_t headerEnd = rawResponse.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        return response;  // 無效響應
    }
    
    std::string headerStr = rawResponse.substr(0, headerEnd);
    std::string bodyStr = rawResponse.substr(headerEnd + 4);
    
    // 2. 解析狀態行
    size_t firstCR = headerStr.find("\r\n");
    std::string statusLine = headerStr.substr(0, firstCR);
    
    // 解析狀態碼（例：HTTP/1.1 200 OK）
    size_t codeStart = statusLine.find(' ') + 1;
    size_t codeEnd = statusLine.find(' ', codeStart);
    std::string codeStr = statusLine.substr(codeStart, codeEnd - codeStart);
    response.statusCode = std::stoi(codeStr);
    
    // 3. 解析響應頭
    std::string headersSection = headerStr.substr(firstCR + 2);
    size_t pos = 0;
    while ((pos = headersSection.find("\r\n")) != std::string::npos) {
        std::string headerLine = headersSection.substr(0, pos);
        size_t colonPos = headerLine.find(':');
        if (colonPos != std::string::npos) {
            std::string key = headerLine.substr(0, colonPos);
            std::string value = headerLine.substr(colonPos + 2);  // 跳過": "
            response.headers[key] = value;
            
            // 檢查是否使用分塊編碼
            if (key == "Transfer-Encoding" && 
                value.find("chunked") != std::string::npos) {
                response.isChunked = true;
            }
        }
        headersSection = headersSection.substr(pos + 2);
    }
    
    // 4. 處理響應體
    if (response.isChunked) {
        // 需要解析分塊編碼
        response.body = decodeChunked(bodyStr);
    } else if (response.headers.find("Content-Length") != response.headers.end()) {
        // 使用Content-Length
        response.body = std::vector<char>(bodyStr.begin(), bodyStr.end());
    } else {
        // 假定到EOF為止
        response.body = std::vector<char>(bodyStr.begin(), bodyStr.end());
    }
    
    return response;
}
