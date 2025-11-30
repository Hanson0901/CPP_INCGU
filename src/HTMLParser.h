// HTMLParser.h
#ifndef HTML_PARSER_H
#define HTML_PARSER_H

#include <string>
#include <vector>
#include <regex>

class HTMLParser {
public:
    // 提取頁面中的所有超連結
    static std::vector<std::string> extractLinks(const std::string& html);
    
    // 提取頁面中的所有圖片
    static std::vector<std::string> extractImages(const std::string& html);
    
    // 提取頁面中的所有資源（CSS, JS等）
    static std::vector<std::string> extractResources(const std::string& html);
    
private:
    // 使用正則表達式提取href屬性
    static std::vector<std::string> extractByRegex(const std::string& html,
                                                  const std::regex& pattern);
};

#endif
