/*TODO: 完成server主体框架
        主线程只负责监听任务到达, 线程池中的
        子线程进行IO与逻辑操作*/

#ifndef WEBSERVER_H
#define WEBSERVER_H
#include "../log/log.h"
#include "../timer/heaptimer.h"
#include "../http/httpconn.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"
#include "epoller.h"

#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>
#include <unordered_map>
class webserver{

public:
    webserver(int port, int trig_mode, int time_out_ms, bool open_linger,
                int sql_port, const char* sql_user, const char* sql_pwd,
                const char* db_name, int sql_conn_pool_num, int thread_num,
                bool open_log, int max_log_queue_size);
    ~webserver();
    void start();

private:
    //设置触发模式
    void initEventMode(int trig_mode);
    //建立listen socket
    bool initListenSocket();

    //处理新建连接
    void dealListen();
    //dealListen的辅助函数, 添加client
    void addClient(int fd, sockaddr_in& client_addr);

    //添加读写任务
    void addReadTask(int fd);
    void addWriteTask(int fd);
    
    //处理读与写任务
    void dealRead(int fd);
    /*NOTE: 关于为什么需要这个process函数, 
        1. 逻辑处理过程
        2. 主要是因为ET模式下每次有数据
        可写的情况下, 需要通过epoll_ctl重新添加EPOLLOUT事件才能触发写
        参考https://blog.csdn.net/whatday/article/details/103472489
    */
    //逻辑处理
    void process(int fd);
    void dealWrite(int fd);


    //事件到达后, 延长超时时间
    void extendTime(int fd);
    //关闭连接
    void closeConn(client_ptr client);
    void sendError(int fd, const char* info);

    static const int MAX_FD = 65536;

    static int setFdNonBlock(int fd);

    int _port;
    bool _open_linger;//控制是否优雅关闭
    int _time_out_ms;
    bool is_close;
    int listen_fd;
    std::unique_ptr<char[]> _src_dir;

    uint32_t listen_event;
    uint32_t conn_event;

    std::unique_ptr<timer_heap> _timer;
    std::unique_ptr<thread_pool> _thread_pool;
    std::unique_ptr<epoller> _epoller;

    std::unordered_map<int, client_ptr> users;
};


#endif