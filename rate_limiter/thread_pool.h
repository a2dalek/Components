#pragma once
#include <thread>
#include <functional>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <future>
#include <queue>

class ThreadPool {

public:
    ThreadPool(int num_threads);

    template <class F, class... Args>
    std::future<std::result_of_t<F(Args...)>> submit_job(F &&f, Args &&...args);

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;

    virtual ~ThreadPool();

private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;

    std::mutex mtx;
    bool should_stop;
    std::condition_variable cv;
};