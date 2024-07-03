#include "epoller.h"
#include <cerrno>
#include <stdio.h>
#include <cstring>
using std::vector;

epoller::epoller(int max_events) : _events(max_events){
    epoll_fd = epoll_create(512);
    assert(epoll_fd >= 0);
}

epoller::~epoller(){
    close(epoll_fd);
}

bool epoller::addFd(int fd, uint32_t event){
    if (fd < 0)
        return false;
    epoll_event tmp_ev{0};
    tmp_ev.data.fd = fd;
    tmp_ev.events = event;
    //BUG:  为什么这里有的时候报错 invalid argument?    已解决, 转webserver.cpp:10
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &tmp_ev) != 0){
        /*TODO:BUG: epoll_ctl failed: Bad file descriptor*/
        fprintf(stderr, "epoll_ctl failed: %s\n", strerror(errno));
        return false;
    }
    return true;
}

bool epoller::modFd(int fd, uint32_t event){
    if (fd < 0)
        return false;
    epoll_event tmp_ev{0};
    tmp_ev.data.fd = fd;
    tmp_ev.events = event;
    return 0 == epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &tmp_ev);
}

bool epoller::delFd(int fd){
    if (fd < 0)
        return false;
    epoll_event tmp_ev{0};
    return 0 == epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &tmp_ev);
}

int epoller::wait(int wait_time_ms){
    return epoll_wait(epoll_fd, &_events[0], _events.size(), wait_time_ms);
}

int epoller::getEventFd(int idx){
    assert(idx >= 0 && idx < _events.size());
    return _events[idx].data.fd;
}

uint32_t epoller::getEventType(int idx){
    assert(idx >= 0 && idx < _events.size());
    return _events[idx].events;
}