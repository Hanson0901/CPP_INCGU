#ifndef URL_FILTER_H
#define URL_FILTER_H

#include <string>
#include <vector>
#include <set>

struct FilterConfig {
    int max_depth;                    // 最大遞迴深度
    bool download_external;           // 是否下載外部網站
    std::string base_host;            // 基礎主機名稱
    std::vector<std::string> allowed_types; // 允許的檔案類型
    long min_file_size;               // 最小檔案大小
    long max_file_size;               // 最大檔案大小
    
    FilterConfig() : max_depth(0), download_external(false), min_file_size(0), max_file_size(-1) {}
};

class URLFilter {
private:
    FilterConfig config;
    
public:
    URLFilter(const FilterConfig& config);
    
    // 檢查URL是否應該被下載
    bool should_download(const std::string& url, int depth, const std::string& current_host);
    
    // 檢查檔案類型是否符合要求
    bool check_file_type(const std::string& url);
    
    // 檢查檔案大小是否符合要求
    bool check_file_size(long file_size);
    
    // 從URL中提取主機名稱
    static std::string extract_host(const std::string& url);
};

#endif
