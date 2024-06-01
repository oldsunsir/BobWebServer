/*DONE: 基于生产者消费者模型完成有限阻塞队列
        方便后续异步日志处理    
    支持操作：
        clear   full    empty   close   size
        capacity    front   back    
        push    pop     */
#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <mutex>
#include <condition_variable>
#include <deque>
#include <sys/time.h>
#include <assert.h>
#include <stdio.h>
template<class T>
class block_queue{

public:
    explicit block_queue(size_t capacity) : _capacity(capacity){
        _is_close = false;
    }
    ~block_queue();
    void clear();
    bool full();
    bool empty();

    void close();
    size_t size();
    size_t capacity();
    T front();
    T back();

    bool push(const T& item);
    bool pop(T& item);
    bool pop(T& item, int timeout);
    
    bool is_close() { return _is_close; }

private:
    size_t _capacity;
    std::deque<T> _deque;
   
    std::mutex _mtx;
    std::condition_variable _produce_cv;
    std::condition_variable _consume_cv;

    bool _is_close;
};

template<class T>
bool block_queue<T>::pop(T& item, int timeout){
    std::unique_lock<std::mutex> locker(_mtx);
    if (!_consume_cv.wait_for(locker, std::chrono::seconds(timeout), [this](){
        return (!_deque.empty()) || (_is_close == true);
        }))
        return false;
    if (_is_close)
        return false;
    item = _deque.front();
    _deque.pop_front();
    _produce_cv.notify_one();
    return true;
}

template<class T>
bool block_queue<T>::pop(T& item){
    std::unique_lock<std::mutex> locker(_mtx);
    _consume_cv.wait(locker, [this](){
        return (!_deque.empty()) || (_is_close == true);
    });
    if (_is_close){
        return false;
    }
    item = _deque.front();

    _deque.pop_front();
    _produce_cv.notify_one();
    return true;
}

template<class T>
bool block_queue<T>::push(const T& item){
    std::unique_lock<std::mutex> locker(_mtx);

/*  BUG:    错误写法, 这里的判断函数里面调用!full(), 会死锁
            因为full需要获取锁...   
    
    TODO:   这里如果队列满了, push应该怎么处理？ 
            这样阻塞等待是否影响本身逻辑主线程  */
    _produce_cv.wait(locker, [this](){
        return ((_deque.size() < _capacity) || (_is_close == true));
    });
    if (_is_close)
        return false;
    _deque.push_back(item);
    _consume_cv.notify_one();
    return true;
}

template<class T>
T block_queue<T>::back(){
    std::unique_lock<std::mutex> locker(_mtx);
    return _deque.back();
}

template<class T>
T block_queue<T>::front(){
    std::unique_lock<std::mutex> locker(_mtx);
    return _deque.front();
}

template<class T>
size_t block_queue<T>::capacity(){
    std::unique_lock<std::mutex> locker(_mtx);
    return _capacity;
}

template<class T>
size_t block_queue<T>::size(){
    std::unique_lock<std::mutex> locker(_mtx);
    return _deque.size();
}

template<class T>
block_queue<T>::~block_queue(){
    close();
}

template<class T>
void block_queue<T>::clear(){
    std::unique_lock<std::mutex> locker(_mtx);
    _deque.clear();
}

template<class T>
bool block_queue<T>::full(){
    std::unique_lock<std::mutex> locker(_mtx);
    return _capacity == _deque.size();
}

template<class T>
bool block_queue<T>::empty(){
    std::unique_lock<std::mutex> locker(_mtx);
    return _deque.empty();
}

template<class T>
void block_queue<T>::close(){
    {
        std::unique_lock<std::mutex> locker(_mtx);
        _deque.clear();
        _is_close = true;
    }
    _produce_cv.notify_all();
    _consume_cv.notify_all();
}
#endif

