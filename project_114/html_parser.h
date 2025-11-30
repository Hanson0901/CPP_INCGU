#ifndef HTML_PARSER_H
#define HTML_PARSER_H

#include <string>
#include <vector>
#include <set>

class HTMLParser {
public:
    // 從HTML內容中提取所有連結
    static std::vector<std::string> extract_links(const std::string& html, 
                                                   const std::string& base_url);
    
    // 從HTML內容中提取所有資源連結（img, css, js等）
    static std::vector<std::string> extract_resources(const std::string& html,
                                                       const std::string& base_url);
    
    // 正規化URL（處理相對路徑）
    static std::string normalize_url(const std::string& url, const std::string& base_url);
    
private:
    // 提取特定標籤的屬性值
    static void extract_tag_attributes(const std::string& html, const std::string& tag,
                                      const std::string& attr, std::vector<std::string>& result,
                                      const std::string& base_url);
    
    // 檢查URL是否為絕對路徑
    static bool is_absolute_url(const std::string& url);
};

#endif
