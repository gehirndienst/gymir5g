/*
 * TaskPerformer.h
 *
 * a class which stores and performs tasks in separate thread and pops outputs in a form of future result
 *
 * Created on: Mar 9, 2023
 *      Author: Nikita Smirnov
 */

#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#ifndef TASKPERFORMER_H
#define TASKPERFORMER_H

class TaskPerformer {
public:
    TaskPerformer() : isStopped_(false), thread_(&TaskPerformer::run, this) {}

    ~TaskPerformer() {
        isStopped_ = true;
        cvar_.notify_all();
        std::queue<std::function<void()>>().swap(taskQueue_);
        thread_.join();
    }

    template<typename F, typename... Args>
    auto addTask(F&& f, Args&&... args) {
        using ReturnType = typename std::result_of<F(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));   
        auto futureResult = task->get_future();
        {
            std::unique_lock<std::mutex> lock(mutex_);
            taskQueue_.emplace([task](){ (*task)(); });
        }
        cvar_.notify_one();
        return futureResult;
    }

private:
    void run() {
        while (!isStopped_) {
            std::unique_lock<std::mutex> lock(mutex_);
            cvar_.wait(lock, [this]() { return !taskQueue_.empty() || isStopped_; });
            if (isStopped_) break;
            auto task = std::move(taskQueue_.front());
            taskQueue_.pop();
            lock.unlock();
            task();
        }
    }

    bool isStopped_;
    std::thread thread_;
    std::queue<std::function<void()>> taskQueue_;
    mutable std::mutex mutex_;
    std::condition_variable cvar_;
};

#endif
