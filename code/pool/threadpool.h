/*TODO: 线程池  */
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <functional>
class thread_pool{

public:
    explicit thread_pool(size_t thread_cnt = 8) : pool_sp(std::make_shared<pool>()){
        for (int i = 0; i < thread_cnt; ++i){
            std::thread([this](){
                while (true){
                    std::unique_lock<std::mutex> locker(pool_sp->mtx);
                    if (!pool_sp->tasks.empty()){
                        auto task = std::move(pool_sp->tasks.front());
                        pool_sp->tasks.pop();
                        locker.unlock();
                        task();
                    }
                    else if (pool_sp->is_close)
                        break;
                    else{
                        pool_sp->cv.wait(locker);
                    }
                }
            }).detach();
        }
    }
    ~thread_pool(){
        if (static_cast<bool> (pool_sp)){
            {
                std::lock_guard<std::mutex> locker(pool_sp->mtx);
                pool_sp->is_close = true;
            }
            pool_sp->cv.notify_all();
        }
    }

    template <class T>
    void addTask(T&& task){
        /*  INFO:为啥要加{}, 不直接在后面写notify_one?*/
        {
            std::lock_guard<std::mutex> locker(pool_sp->mtx);
            pool_sp->tasks.emplace(std::forward<T>(task));
        }
        pool_sp->cv.notify_one();
    }
private:
    struct pool{
        std::mutex mtx;
        std::condition_variable cv;
        std::queue<std::function<void()>> tasks;
        bool is_close{false};
    };
    std::shared_ptr<pool> pool_sp;
};
#endif