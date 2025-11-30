#include "http_client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <iostream>

HTTPClient::HTTPClient() : sockfd(-1), port(80), connected(false) {}

HTTPClient::~HTTPClient() {
    disconnect();
}

bool HTTPClient::parse_url(const std::string& url, std::string& protocol, 
                          std::string& host, int& port, std::string& path) {
    size_t pos = url.find("://");
    if (pos == std::string::npos) {
        protocol = "http";
        pos = 0;
    } else {
        protocol = url.substr(0, pos);
        pos += 3;
    }
    
    size_t host_end = url.find('/', pos);
    if (host_end == std::string::npos) {
        host = url.substr(pos);
        path = "/";
    } else {
        host = url.substr(pos, host_end - pos);
        path = url.substr(host_end);
    }
    
    // 檢查是否有port
    size_t port_pos = host.find(':');
    if (port_pos != std::string::npos) {
        port = std::stoi(host.substr(port_pos + 1));
        host = host.substr(0, port_pos);
    } else {
        port = (protocol == "https") ? 443 : 80;
    }
    
    return true;
}

bool HTTPClient::connect_to_server() {
    if (connected) return true;
    
    struct sockaddr_in server_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return false;
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    struct hostent* server = gethostbyname(host.c_str());
    if (server == NULL) {
        std::cerr << "Error resolving host: " << host << std::endl;
        close(sockfd);
        return false;
    }
    
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error connecting to server" << std::endl;
        close(sockfd);
        return false;
    }
    
    connected = true;
    return true;
}

std::string HTTPClient::build_request(const std::string& method, const std::string& path,
                                     const std::map<std::string, std::string>& headers) {
    std::stringstream request;
    request << method << " " << path << " HTTP/1.1\r\n";
    request << "Host: " << host << "\r\n";
    request << "User-Agent: Mozilla/5.0 (CourseProject/1.0)\r\n";
    request << "Accept: */*\r\n";
    request << "Connection: close\r\n";
    
    for (const auto& header : headers) {
        request << header.first << ": " << header.second << "\r\n";
    }
    
    request << "\r\n";
    return request.str();
}

void HTTPClient::parse_response(const std::string& raw_response, HTTPResponse& response) {
    std::istringstream stream(raw_response);
    std::string line;
    
    // 解析狀態行
    std::getline(stream, line);
    std::istringstream status_line(line);
    status_line >> response.version >> response.status_code;
    std::getline(status_line, response.status_text);
    
    // 解析標頭
    response.content_length = -1;
    response.is_chunked = false;
    
    while (std::getline(stream, line) && line != "\r") {
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            
            // 移除前後空格
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            
            response.headers[key] = value;
            
            if (key == "Content-Length") {
                response.content_length = std::stol(value);
            } else if (key == "Transfer-Encoding" && value.find("chunked") != std::string::npos) {
                response.is_chunked = true;
            }
        }
    }
    
    // 解析主體
    response.body.clear();
    if (response.is_chunked) {
        // 處理chunked encoding
        while (std::getline(stream, line)) {
            // 跳過chunk size行
            if (line.find(';') != std::string::npos) {
                line = line.substr(0, line.find(';'));
            }
            int chunk_size = std::stoi(line, nullptr, 16);
            if (chunk_size == 0) break;
            
            std::vector<char> chunk(chunk_size);
            stream.read(chunk.data(), chunk_size);
            response.body.insert(response.body.end(), chunk.begin(), chunk.end());
            
            // 跳過CRLF
            stream.ignore(2);
        }
    } else if (response.content_length > 0) {
        // 直接讀取指定長度
        std::vector<char> body(response.content_length);
        stream.read(body.data(), response.content_length);
        response.body = body;
    } else {
        // 讀取到結束
        std::vector<char> body(std::istreambuf_iterator<char>(stream), {});
        response.body = body;
    }
}

bool HTTPClient::get(const std::string& url, HTTPResponse& response) {
    std::string protocol, path;
    if (!parse_url(url, protocol, host, port, path)) {
        return false;
    }
    
    if (protocol != "http") {
        std::cerr << "Only HTTP protocol is supported" << std::endl;
        return false;
    }
    
    if (!connect_to_server()) {
        return false;
    }
    
    std::map<std::string, std::string> headers;
    std::string request = build_request("GET", path, headers);
    
    // 發送請求
    if (send(sockfd, request.c_str(), request.length(), 0) < 0) {
        std::cerr << "Error sending request" << std::endl;
        return false;
    }
    
    // 接收回應
    char buffer[4096];
    std::string raw_response;
    int bytes_received;
    
    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        raw_response += buffer;
    }
    
    if (bytes_received < 0) {
        std::cerr << "Error receiving response" << std::endl;
        return false;
    }
    
    parse_response(raw_response, response);
    disconnect();
    
    return response.status_code == 200;
}

bool HTTPClient::head(const std::string& url, HTTPResponse& response) {
    std::string protocol, path;
    if (!parse_url(url, protocol, host, port, path)) {
        return false;
    }
    
    if (protocol != "http") {
        std::cerr << "Only HTTP protocol is supported" << std::endl;
        return false;
    }
    
    if (!connect_to_server()) {
        return false;
    }
    
    std::map<std::string, std::string> headers;
    std::string request = build_request("HEAD", path, headers);
    
    if (send(sockfd, request.c_str(), request.length(), 0) < 0) {
        std::cerr << "Error sending request" << std::endl;
        return false;
    }
    
    char buffer[4096];
    std::string raw_response;
    int bytes_received;
    
    if ((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        raw_response += buffer;
    }
    
    parse_response(raw_response, response);
    disconnect();
    
    return response.status_code == 200;
}

void HTTPClient::disconnect() {
    if (sockfd >= 0) {
        close(sockfd);
        sockfd = -1;
        connected = false;
    }
}
