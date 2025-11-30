#include "url_filter.h"
#include <algorithm>
#include <sstream>

URLFilter::URLFilter(const FilterConfig& config) : config(config) {}

std::string URLFilter::extract_host(const std::string& url) {
    size_t pos = url.find("://");
    if (pos == std::string::npos) return "";
    
    std::string rest = url.substr(pos + 3);
    size_t slash_pos = rest.find('/');
    if (slash_pos != std::string::npos) {
        return rest.substr(0, slash_pos);
    }
    return rest;
}

bool URLFilter::should_download(const std::string& url, int depth, const std::string& current_host) {
    // 檢查深度
    if (depth > config.max_depth) {
        return false;
    }
    
    // 檢查是否為外部網站
    if (!config.download_external) {
        std::string url_host = extract_host(url);
        if (!url_host.empty() && url_host != config.base_host && url_host != current_host) {
            return false;
        }
    }
    
    // 檢查檔案類型
    if (!config.allowed_types.empty() && !check_file_type(url)) {
        return false;
    }
    
    return true;
}

bool URLFilter::check_file_type(const std::string& url) {
    if (config.allowed_types.empty()) return true;
    
    size_t dot_pos = url.find_last_of('.');
    if (dot_pos == std::string::npos) return false;
    
    std::string extension = url.substr(dot_pos + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    for (const std::string& allowed_type : config.allowed_types) {
        if (extension == allowed_type) {
            return true;
        }
    }
    
    return false;
}

bool URLFilter::check_file_size(long file_size) {
    if (config.min_file_size > 0 && file_size < config.min_file_size) {
        return false;
    }
    
    if (config.max_file_size > 0 && file_size > config.max_file_size) {
        return false;
    }
    
    return true;
}
