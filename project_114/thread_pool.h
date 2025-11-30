#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
    std::atomic<size_t> active_tasks;
    
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();
    
    // 添加任務到佇列
    void enqueue(std::function<void()> task);
    
    // 等待所有任務完成
    void wait_all();
    
    // 獲取活躍任務數量
    size_t get_active_tasks() const;
};

#endif
