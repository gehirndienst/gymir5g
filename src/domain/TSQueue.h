/*
 * TSQueue.h
 *
 * a thread-safe, shared and fixed-size (opt.) std::queue overriding
 *
 * Created on: Apr 22, 2022
 *      Author: Nikita Smirnov
 */

#ifndef TSQUEUE_H
#define TSQUEUE_H

#include <condition_variable>
#include <mutex>
#include <shared_mutex>
#include <queue>
#include <thread>

#pragma once

template <class T>
class TSQueue {
public:
    TSQueue(int limitSize = 0) : limitSize_(limitSize) {};

    TSQueue(const TSQueue<T> &) = delete;

    TSQueue(TSQueue<T>&& other) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        queue_ = std::move(other.queue_);
    }

    ~TSQueue() {};

    void waitPop(T& elem, int milliseconds) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        while (queue_.empty()) {
            if (milliseconds > 0) {
            auto timeout = std::chrono::milliseconds(milliseconds);
            if (condvar_.wait_for(lock, timeout) == std::cv_status::timeout) {
                elem = T();
                return;
            } 
            } else {
            condvar_.wait(lock);
            }
        }
        elem = queue_.front();
        queue_.pop();
    }

    bool tryPop(T& elem) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        } 
        elem = queue_.front();
        queue_.pop();
        return true;
    }

    void push(const T& elem) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        queue_.push(elem);
        lock.unlock();
        condvar_.notify_one();
    }
  
    void push(T&& elem) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        queue_.push(std::move(elem));
        lock.unlock();
        condvar_.notify_one();
    }

    bool empty() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return queue_.empty();
    }

    int size() {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return queue_.size();
    }

    bool isOverflowed() {
        return limitSize_ > 0 ? size() >= limitSize_ : false;
    }

    int getLimitSize() {
        return limitSize_;
    }

    void setLimitSize(int limitSize) {
        limitSize_ = limitSize;
    }

    void clear() {
        std::queue<T>().swap(queue_);
    }
 
 private:
    int limitSize_;
    mutable std::shared_mutex mutex_;
    std::condition_variable_any condvar_;
    std::queue<T> queue_;
};

#endif