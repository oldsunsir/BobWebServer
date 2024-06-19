#include "epoller.h"
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
    return 0 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &tmp_ev);
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