#include "rate_limiter/thread_pool.h"

ThreadPool::ThreadPool(int num_threads) : should_stop(false) {
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
}

template <class F, class... Args>
std::future<std::result_of_t<F(Args...)>> ThreadPool::submit_job(F &&f, Args &&...args) {
    using return_type = std::result_of_t<F(Args...)>;

    auto task = std::make_shared<std::packaged_task<return_type>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    std::unique_lock<std::mutex> lock(mtx);
    if (should_stop) {

    }
    tasks.emplace([task] -> void{ (*task)(); });
    cv.notify_one();
    return res;
}

ThreadPool::~ThreadPool()
{
    /* stop thread pool, and notify all threads to finish the remained tasks. */
    {
        std::unique_lock<std::mutex> lock(mtx);
        should_stop = true;
    }
    cv.notify_all();
    for (auto &thread : threads)
        thread.join();
}