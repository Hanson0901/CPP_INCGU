#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <string>
#include <vector>
#include <map>
#include <netdb.h> 
struct HTTPResponse {
    std::string version;
    int status_code;
    std::string status_text;
    std::map<std::string, std::string> headers;
    std::vector<char> body;
    bool is_chunked;
    long content_length;
};

class HTTPClient {
private:
    int sockfd;
    std::string host;
    int port;
    bool connected;
    
    bool connect_to_server();
    void parse_response(const std::string& raw_response, HTTPResponse& response);
    std::string build_request(const std::string& method, const std::string& path, 
                             const std::map<std::string, std::string>& headers);
    
public:
    HTTPClient();
    ~HTTPClient();
    
    // 發送HTTP GET請求
    bool get(const std::string& url, HTTPResponse& response);
    
    // 檢查URL是否存在（HEAD請求）
    bool head(const std::string& url, HTTPResponse& response);
    
    // 斷開連接
    void disconnect();
    
    // 解析URL
    static bool parse_url(const std::string& url, std::string& protocol, 
                         std::string& host, int& port, std::string& path);
};

#endif
