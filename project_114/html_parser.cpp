#include "html_parser.h"
#include <regex>
#include <algorithm>

std::vector<std::string> HTMLParser::extract_links(const std::string& html, 
                                                   const std::string& base_url) {
    std::vector<std::string> links;
    extract_tag_attributes(html, "a", "href", links, base_url);
    return links;
}

std::vector<std::string> HTMLParser::extract_resources(const std::string& html,
                                                       const std::string& base_url) {
    std::vector<std::string> resources;
    
    // 提取圖片
    extract_tag_attributes(html, "img", "src", resources, base_url);
    
    // 提取CSS
    extract_tag_attributes(html, "link", "href", resources, base_url);
    
    // 提取JavaScript
    extract_tag_attributes(html, "script", "src", resources, base_url);
    
    // 提取其他資源
    extract_tag_attributes(html, "source", "src", resources, base_url);
    extract_tag_attributes(html, "iframe", "src", resources, base_url);
    
    return resources;
}

void HTMLParser::extract_tag_attributes(const std::string& html, const std::string& tag,
                                       const std::string& attr, std::vector<std::string>& result,
                                       const std::string& base_url) {
    // 使用regex提取標籤
    std::regex tag_regex("<" + tag + "\\s+[^>]*" + attr + "\\s*=\\s*[\"']?([^\"'\\s>]+)[\"']?[^>]*>",
                        std::regex::icase);
    std::sregex_iterator iter(html.begin(), html.end(), tag_regex);
    std::sregex_iterator end;
    
    for (; iter != end; ++iter) {
        std::string url = (*iter)[1].str();
        std::string normalized_url = normalize_url(url, base_url);
        if (!normalized_url.empty()) {
            result.push_back(normalized_url);
        }
    }
}

std::string HTMLParser::normalize_url(const std::string& url, const std::string& base_url) {
    if (url.empty() || url[0] == '#' || url.find("javascript:") == 0 || url.find("mailto:") == 0) {
        return "";
    }
    
    if (is_absolute_url(url)) {
        return url;
    }
    
    // 處理相對路徑
    std::string protocol, host, path;
    size_t pos = base_url.find("://");
    if (pos != std::string::npos) {
        protocol = base_url.substr(0, pos + 3);
        std::string rest = base_url.substr(pos + 3);
        size_t slash_pos = rest.find('/');
        if (slash_pos != std::string::npos) {
            host = rest.substr(0, slash_pos);
            path = rest.substr(slash_pos);
        } else {
            host = rest;
            path = "/";
        }
    } else {
        return "";
    }
    
    if (url.substr(0, 2) == "./") {
        // 相對路徑
        size_t last_slash = path.find_last_of('/');
        std::string base_path = (last_slash != std::string::npos) ? path.substr(0, last_slash + 1) : "/";
        return protocol + host + base_path + url.substr(2);
    } else if (url[0] == '/') {
        // 絕對路徑
        return protocol + host + url;
    } else {
        // 相對路徑
        size_t last_slash = path.find_last_of('/');
        std::string base_path = (last_slash != std::string::npos) ? path.substr(0, last_slash + 1) : "/";
        return protocol + host + base_path + url;
    }
}

bool HTMLParser::is_absolute_url(const std::string& url) {
    return url.find("http://") == 0 || url.find("https://") == 0 ||
           url.find("ftp://") == 0 || url.find("//") == 0;
}
