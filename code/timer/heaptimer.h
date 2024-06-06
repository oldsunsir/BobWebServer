/*  TODO:   基于小根堆的定时器  */
#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H


#include <netinet/in.h>
#include <functional>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <memory>
#include <time.h>
#include <chrono>
#include <string>
#include "../log/log.h"

struct client_data;
class timer_node;

/*  NOTE:应该用shared_ptr, 比如client_data中有一份timer_node_ptr
                timer_heap的array中也有一份
            但这样client_data和timer_node会存在循环引用,
            故在client_data中使用weak_ptr指向timer_node    */
typedef std::shared_ptr<client_data> client_ptr;
typedef std::weak_ptr<timer_node> timer_node_weak_ptr;
typedef std::shared_ptr<timer_node> timer_node_ptr; 

typedef std::function<void(client_ptr)> time_out_cb;
typedef std::chrono::high_resolution_clock timer_clock;
typedef std::chrono::milliseconds ms;
typedef timer_clock::time_point time_stamp;

struct client_data{
    sockaddr_in address;
    int sockfd;
    timer_node_weak_ptr timer;
};

class timer_node{
public:
    explicit timer_node(int delay){
        expires = timer_clock::now() + ms(delay);
    }

    friend std::time_t log_out_expires(const timer_node& tn){
        std::time_t time = std::chrono::system_clock::to_time_t(tn.expires);
        return time;
    }
public:
    time_stamp expires;
    time_out_cb cb_func;
    client_ptr user_data;
};

class timer_heap{
public:
    explicit timer_heap(int cap);

    ~timer_heap();

    void add_timer(timer_node_ptr timer_node);

    void del_timer(timer_node_ptr timer_node);

    void adjust(timer_node_ptr timer_node, int new_expires);

    /*  获取堆顶部定时器    */
    timer_node_ptr top() const;
    /*  删除堆顶部定时器    */
    void pop_timer();
    /*  心搏函数    */
    void tick();

    bool empty() const{
        return cur_size == 0;
    }

    int size() const{
        return cur_size;
    }
    int cap() const{
        return capacity;
    }
private:
    /*  下沉, 确保以hole为根的子树为最小堆  */
    void percolate_down(int hole);  
    /*  上浮, 确保hole满足小根堆性质    */
    void percolate_up(int hole);

private:
    std::vector<timer_node_ptr> array;  //堆数组
    int capacity;                   //堆容量
    int cur_size;                   //堆当前大小
    /*  NOTE:暂时不用map记录ptr的位置, hash不好写    */
    // std::unordered_map<timer_node_ptr, size_t> ref; //获取ptr对应的idx
};
#endif