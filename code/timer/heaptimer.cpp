#include "heaptimer.h"

timer_heap::timer_heap(int cap) : capacity(cap), cur_size(0){
    array.reserve(capacity);
}

timer_heap::~timer_heap(){
    array.clear();
}

timer_node_ptr timer_heap::top() const{
    if (empty())
        return nullptr;
    return array[0];
};

void timer_heap::pop_timer(){
    if (empty()) 
        return;
    if (array[0]){
        array[0] = array[--cur_size];
        percolate_down(0);
    }
    /*  队尾元素删除    */
    array.pop_back();
}

void timer_heap::add_timer(timer_node_ptr timer_node){
    if (!timer_node){
        return;
    }
    /*  新节点,  队尾插入, 再上浮    */
    /*  TODO:   未考虑timer已存在的情况*/
    array.push_back(timer_node);
    int hole = cur_size++;  //获得新插入节点的idx
    percolate_up(hole);
}

void timer_heap::del_timer(timer_node_ptr timer_node){
    auto timer_it = std::find(array.cbegin(), array.cend(), timer_node);
    if (timer_it == array.cend())
        return;
    int hole = std::distance(array.cbegin(), timer_it);
    array[hole] = array[--cur_size];
    array.pop_back();
    percolate_down(hole);
    percolate_up(hole);
}

void timer_heap::adjust(timer_node_ptr timer_node, int new_expires){
    auto timer_it = std::find(array.cbegin(), array.cend(), timer_node);
    if (timer_it == array.cend())
        return;
    int hole = std::distance(array. cbegin(), timer_it);
    array[hole]->expires = timer_clock::now() + ms(new_expires);
    percolate_down(hole);
}

void timer_heap::tick(){
    
    while (!empty()){
        timer_node_ptr tmp = array[0];
        if (std::chrono::duration_cast<ms>(tmp->expires - timer_clock::now()).count() > 0){
            break;
        }
        tmp->cb_func(tmp->user_data);
        pop_timer();
    }
}

int timer_heap::getNextTick(){
    int res = -1;
    tick();
    if (!empty()){
        res = std::chrono::duration_cast<ms>(array.front()->expires - timer_clock::now()).count();
        if (res < 0)
            res = 0;
    }
    return res;
}

void timer_heap::percolate_down(int hole){
    timer_node_ptr tmp = array[hole];
    int child = 0;
    for (; hole * 2 + 1 < cur_size; hole = child){
        child = hole * 2 + 1;
        if (child + 1 < cur_size && array[child+1]->expires < array[child]->expires){
            child = child+1;
        }
        if (tmp->expires < array[child]->expires)
            break;
        array[hole] = array[child];
    }
    array[hole] = tmp;
}

void timer_heap::percolate_up(int hole){
    timer_node_ptr tmp = array[hole];
    int parent = 0;
    for (; hole > 0; hole = parent){
        parent = (hole - 1) / 2;
        if (tmp->expires >= array[parent]->expires)
            break;
        array[hole] = array[parent];
    }
    array[hole] = tmp;
}