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
    ThreadPool(int num_threads) : should_stop(false) {
        for (unsigned i = 0; i < num_threads; ++i) {
            std::thread worker([this]() {
                while (true) {
                    std::function<void()> task;
                    std::unique_lock<std::mutex> lock(mtx);

                    cv.wait(lock, [this]() {
                        return should_stop || !tasks.empty();
                    });

                    if (should_stop && tasks.empty())
                        return;

                    task = std::move(tasks.front());
                    tasks.pop();

                    task();
                }
            });

            threads.emplace_back(std::move(worker));
        }
    };

    template <class F, class... Args>
    std::future<std::result_of_t<F(Args...)>> submit_job(F &&f, Args &&...args) {
        using return_type = std::result_of_t<F(Args...)>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();
        std::unique_lock<std::mutex> lock(mtx);
        if (should_stop) {

        }

        tasks.emplace([task]() -> void{ 
            (*task)(); 
        });

        cv.notify_one();
        return res;
    }

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;

    virtual ~ThreadPool() {
        /* stop thread pool, and notify all threads to finish the remained tasks. */
        std::unique_lock<std::mutex> lock(mtx);
        should_stop = true;

        cv.notify_all();
        for (auto &thread : threads)
            thread.join();
    }

private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;

    std::mutex mtx;
    bool should_stop;
    std::condition_variable cv;

};