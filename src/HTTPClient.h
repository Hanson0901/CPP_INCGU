// HTTPClient.h
#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <string>
#include <map>
#include <vector>

struct HTTPResponse {
    int statusCode;
    std::map<std::string, std::string> headers;
    std::vector<char> body;
    bool isChunked;
};

struct HTTPRequest {
    std::string method;        // GET, HEAD
    std::string url;
    std::string host;
    int port;
    std::map<std::string, std::string> headers;
};

class HTTPClient {
private:
    static const int BUFFER_SIZE = 4096;
    
    // Socket操作（Linux/Unix）
    int createSocket(const std::string& host, int port);
    bool sendRequest(int socket, const HTTPRequest& req);
    HTTPResponse receiveResponse(int socket);
    
    // HTTP解析
    std::string parseStatusLine(const std::string& line);
    std::map<std::string, std::string> parseHeaders(const std::string& headerStr);
    std::vector<char> parseBody(const std::string& headerStr, 
                                 const std::vector<char>& rawData);
    
public:
    HTTPClient();
    ~HTTPClient();
    
    HTTPResponse GET(const std::string& url, bool followRedirect = true);
    HTTPResponse HEAD(const std::string& url);
    
    // 流式下載（用於大檔案）
    bool downloadToFile(const std::string& url, const std::string& filepath,
                       int* progress = nullptr);
};

#endif
