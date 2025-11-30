// URLParser.h
#ifndef URL_PARSER_H
#define URL_PARSER_H

#include <string>

struct URL {
    std::string protocol;      // http, https
    std::string host;
    int port;
    std::string path;
    std::string query;
    std::string fragment;
    
    // 獲取相對路徑（用於GET請求）
    std::string getRequestPath() const {
        std::string result = path;
        if (!query.empty()) result += "?" + query;
        if (!fragment.empty()) result += "#" + fragment;
        return result;
    }
};

class URLParser {
public:
    static URL parse(const std::string& urlStr);
    
    // 將相對URL轉換為絕對URL
    static std::string resolveRelativeURL(const std::string& baseURL,
                                         const std::string& relativeURL);
    
private:
    static std::string urlDecode(const std::string& encoded);
};

#endif
