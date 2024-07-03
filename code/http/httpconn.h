/*TODO: 基于http_request与http_response实现http_conn类*/
#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include "httprequest.h"
#include "httpresponse.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <atomic>

class http_conn{
public:
    http_conn();
    ~http_conn();
    void init(int fd, const sockaddr_in& addr);
    void close();

    int getFd() const;
    int getPort() const;
    char* getIP() const;
    sockaddr_in getAddr() const;

    ssize_t read(int* save_errno);
    bool process();
    ssize_t write(int* save_errno);

    bool isKeepAlive() const{
        return _request.isKeepAlive();
    }
    
    size_t toWriteBytes() const{
        return iov[0].iov_len + iov[1].iov_len;
    }
    static bool is_et;
    static const char* src_dir;
    /*NOTE: 多个线程进行http连接, 需要原子操作*/
    static std::atomic<int> user_cnt;

private:
    http_request _request;
    http_response _response;
    
    buffer read_buff;
    buffer write_buff;

    /*INFO: 每个连接都有对应的connect fd和client addr*/
    int _fd;
    struct sockaddr_in _addr;

    /*INFO: iovec分区写*/
    int iov_cnt;
    struct iovec iov[2];

    /*INFO: _close:防止重复关闭*/
    bool _is_close;
};
#endif