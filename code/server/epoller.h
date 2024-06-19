/*TODO: 对epoll api进行简单封装*/
#ifndef EPOLLER_H
#define EPOLLER_H
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>
class epoller{

public:
    explicit epoller (int max_events);
    ~epoller();

    bool addFd(int fd, uint32_t event);

    bool modFd(int fd, uint32_t event);

    bool delFd(int fd);

    int wait(int wait_time_ms);

    int getEventFd(int idx);

    uint32_t getEventType(int idx);
private:
    std::vector<epoll_event> _events;
    int epoll_fd;
};

#endif