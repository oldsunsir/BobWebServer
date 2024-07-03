#include "webserver.h"

webserver::webserver(int port, int trig_mode, int time_out_ms, bool open_linger,
                int sql_port, const char* sql_user, const char* sql_pwd,
                const char* db_name, int sql_conn_pool_num, int thread_num,
                bool open_log, int max_log_queue_size)
        : _port(port), _time_out_ms(time_out_ms), 
        is_close(false), _open_linger(open_linger){
    
    /*BUG:  一开始我没有初始化listen_event
            这导致后面add listen_fd的时候, 有时报错
            epoll_ctl invalid_argument。。。。
            就是因为我不初始化, 导致listen_event是个随机值, 我真吐血了*/
    listen_event = 0, conn_event = 0;
    initEventMode(trig_mode);
    sql_pool::instance()->init("localhost", sql_port, sql_user, sql_pwd,
                                db_name, sql_conn_pool_num);
    _thread_pool = std::make_unique<thread_pool>(thread_num);
    _timer = std::make_unique<timer_heap>(64);
    _epoller = std::make_unique<epoller>(1024);

    _src_dir = std::make_unique<char[]>(256);
    assert(getcwd(_src_dir.get(), 256) != nullptr);
    strncat(_src_dir.get(), "/../resources", 16);

    http_conn::user_cnt = 0;
    http_conn::src_dir = _src_dir.get();

    if (open_log){
        log::getInstance()->init("./log", ".log", max_log_queue_size);
    }
    if (!initListenSocket()){
        is_close = true;
        LOG_ERROR("========== Server init error!=========="); 
    }
    else{
        LOG_INFO("========== Server init ==========");
        LOG_INFO("Port:%d, OpenLinger: %s", _port, _open_linger? "true":"false");
        LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                        (listen_event & EPOLLET ? "ET": "LT"),
                        (conn_event & EPOLLET ? "ET": "LT"));
        LOG_INFO("srcDir: %s", http_conn::src_dir);
        LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", sql_conn_pool_num, thread_num);
    }

}

webserver::~webserver(){
    _epoller->delFd(listen_fd);
    close(listen_fd);
    is_close = true;
    sql_pool::instance()->closePool();
}

void webserver::start(){
    int wait_time_ms = -1;
    while (!is_close){
        if (_time_out_ms > 0){
            wait_time_ms = _timer->getNextTick();
        }
        int event_cnt = _epoller->wait(wait_time_ms);
        for (int i = 0; i < event_cnt; ++i){
            int event_fd = _epoller->getEventFd(i);
            uint32_t events = _epoller->getEventType(i);
            if (event_fd == listen_fd && (events & EPOLLIN)){
                dealListen();
            }
            else if (events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)){
                assert(users.count(event_fd));
                closeConn(users[event_fd]);
            }
            else if (events & EPOLLIN){
                assert(users.count(event_fd));
                addReadTask(event_fd);
            }
            else if (events & EPOLLOUT){
                assert(users.count(event_fd));
                addWriteTask(event_fd);
            }
            else{
                LOG_ERROR("Unexpected event occurs!")
            }
        }
    }
}
void webserver::initEventMode(int trig_mode){
    conn_event = EPOLLRDHUP | EPOLLHUP | EPOLLERR;
    switch (trig_mode){
        case 0:
            break;
        case 1:
            listen_event |= EPOLLET;
            break;
        case 2:
            conn_event |= EPOLLET;
            break;
        case 3:
            listen_event |= EPOLLET;
            conn_event |= EPOLLET;
            break;
        default:
            listen_event |= EPOLLET;
            conn_event |= EPOLLET;
    }
    http_conn::is_et = conn_event&EPOLLET;
}

bool webserver::initListenSocket(){
    //INFO: SO_LINGER优雅关闭参考https://kiosk007.top/post/time_wait-%E9%97%AE%E9%A2%98%E8%A7%A3%E5%86%B3/
    //INFO: 监听socket的端口复用问题参考https://www.xiaolincoding.com/network/3_tcp/port.html#%E5%AE%A2%E6%88%B7%E7%AB%AF%E7%9A%84%E7%AB%AF%E5%8F%A3%E5%8F%AF%E4%BB%A5%E9%87%8D%E5%A4%8D%E4%BD%BF%E7%94%A8%E5%90%97

    int ret = 0;
    struct sockaddr_in addr;
    if (_port < 1024 || _port > 65535){
        LOG_ERROR("Port:%d error!", _port)
        return false;
    }

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(_port);

    listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0){
        LOG_ERROR("Create ListenSocket error!");
        return false;
    }

    /*NOTE: 优雅关闭*/
    //BUG:  对一个监听socket设置linger有用么?
    linger opt_linger = {0};
    if (_open_linger){
        opt_linger.l_onoff = 1;
        opt_linger.l_linger = 1;
    }
    ret = setsockopt(listen_fd, SOL_SOCKET, SO_LINGER, reinterpret_cast<const void*>(&opt_linger), sizeof(opt_linger));
    if (ret < 0){
        close(listen_fd);
        LOG_ERROR("Init Linger error!");
        return false;
    }

    /*NOTE: 监听socket端口复用*/
    int opt_val = 1;
    ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const void*>(&opt_val), sizeof(opt_val));
    if (ret < 0){
        close(listen_fd);
        LOG_ERROR("Set Socket ReuseAddr error!")
        return false;
    }
    //NOTE: 关于这里为什么使用reinterpret_cast进行指针转换
    //INFO: https://www.cnblogs.com/Allen-rg/p/6999360.html
    ret = bind(listen_fd, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr));
    if (ret < 0){
        close(listen_fd);
        LOG_ERROR("Bind Port:%d error!", _port);
        return false;
    }
    
    ret = listen(listen_fd, 5);
    if (ret < 0){
        close(listen_fd);
        LOG_ERROR("Listen Port:%d error!", _port);
        return false;
    }

    ret = _epoller->addFd(listen_fd, listen_event | EPOLLIN);
    if (ret == 0){
        close(listen_fd);
        LOG_ERROR("Add Listen Socket:%d error!", listen_fd);
        return false;
    }

    setFdNonBlock(listen_fd);
    //NOTE: 为什么监听socket要设置成非阻塞
    //https://blog.csdn.net/xp178171640/article/details/105944038

    LOG_INFO("Server port:%d", _port)
    return true;
}

void webserver::dealListen(){
    sockaddr_in client_addr;
    socklen_t client_addr_length = sizeof(client_addr);
    bzero(&client_addr, sizeof(client_addr));

    do {
        int conn_fd = accept(listen_fd, reinterpret_cast<sockaddr*>(&client_addr),&client_addr_length);
        if (conn_fd < 0)
            return;
        if (http_conn::user_cnt >= MAX_FD){
            sendError(conn_fd, "Server Busy!");
            LOG_WARN("Client is full!")
            return;
        }
        addClient(conn_fd, client_addr);
    }while(listen_event & EPOLLET);
}

void webserver::addClient(int fd, sockaddr_in& client_addr){
    assert(fd > 0);
    users[fd] = std::make_shared<client_data>();
    users[fd]->address = client_addr;
    users[fd]->sockfd = fd;

    users[fd]->conn_ptr = std::make_shared<http_conn>();
    users[fd]->conn_ptr->init(fd, client_addr);

    if (_time_out_ms > 0){
        timer_node_ptr client_timer_node = std::make_shared<timer_node>(_time_out_ms);
        client_timer_node->user_data = users[fd];
        client_timer_node->cb_func = std::bind(&webserver::closeConn, this, std::placeholders::_1);

        users[fd]->timer = client_timer_node;
        LOG_ERROR("正在添加第[%d]个timer_node", _timer->size())
        _timer->add_timer(client_timer_node);
    }
    //NOTE: 记住设置为非阻塞
    setFdNonBlock(fd);
    _epoller->addFd(fd, conn_event | EPOLLIN);
    LOG_INFO("Client [%d] In!", users[fd]->sockfd);
    return;
}

void webserver::addReadTask(int fd){
    extendTime(fd);
    _thread_pool->addTask(std::bind(&webserver::dealRead, this, fd));
}

void webserver::addWriteTask(int fd){
    extendTime(fd);
    _thread_pool->addTask(std::bind(&webserver::dealWrite, this, fd));
}

void webserver::dealRead(int fd){
    assert(users[fd] != nullptr);
    int save_errno = 0;
    ssize_t ret = -1;

    ret = users[fd]->conn_ptr->read(&save_errno);
    if (ret <= 0 && save_errno != EAGAIN && save_errno != EWOULDBLOCK){
        closeConn(users[fd]);
        return;
    }
    process(fd);
}

void webserver::process(int fd){
    if (users[fd]->conn_ptr->process()){
        _epoller->modFd(fd, conn_event | EPOLLOUT);
    }
    else{
        _epoller->modFd(fd, conn_event | EPOLLIN);
    }
}

void webserver::dealWrite(int fd){
    assert(users[fd] != nullptr);
    int save_errno = 0;
    ssize_t ret = -1;

    ret = users[fd]->conn_ptr->write(&save_errno);
    //NOTE: 表示读出错
    if (ret <= 0 && save_errno != EAGAIN && save_errno != EWOULDBLOCK){
        closeConn(users[fd]);
        return;
    }
    /*BUG:  在传输视频时, 数据大, 即使没有写完, 也会返回-1并设置errno为EAGAIN,
            所以在外面需要再判断一下是否是真的写完了数据*/
    if (users[fd]->conn_ptr->toWriteBytes() == 0){
        if (users[fd]->conn_ptr->isKeepAlive()){
            process(fd);
            return;
        }
    }
    else{
        _epoller->modFd(fd, conn_event | EPOLLOUT);
        return;
    }
    closeConn(users[fd]);
}

void webserver::extendTime(int fd){
    if (_time_out_ms > 0){
        assert(users[fd] != nullptr);
        _timer->adjust(users[fd]->timer.lock(), _time_out_ms);
    }
}

void webserver::closeConn(client_ptr client){
    assert(client != nullptr);
    LOG_INFO("Client [%d] quit!", client->sockfd)
    _epoller->delFd(client->sockfd);
    /*BUG:  为什么这里erase后出问题? 
            这里erase后, closeConn结束就会对client进行析构, 原因还没分析清楚。。
            
            但是不erase也没有太大影响, 因为conn_fd会分配到之前close掉的fd, 这样在addClient时,
            通过users[fd] = std::make_shared<client_data>();
            仍然可以析构掉前一个用这个fd的客户分配的client_data内存(因为没人引用他了), 并不会损耗太多内存*/
    // users.erase(client->sockfd);
    // _timer->del_timer(client->timer.lock());
    // LOG_INFO("删除user[%d]后, 当前timer_heap size[%d]", client->sockfd, _timer->size())
    // /*BUG:  我觉得这里一定要把close放在最后处理
    //         否则可能先close了, 然后新的连接建立了, 用了关闭的那个的fd
    //         结果接着再erase就错了*/
    // LOG_INFO("client_ptr [%d] 引用计数为[%d]", client->sockfd, client.use_count())
    // LOG_INFO("client_ptr_conn [%d] 引用计数为[%d]", client->sockfd, client->conn_ptr.use_count())
    users[client->sockfd]->conn_ptr->close();
    // client->conn_ptr->close();
}

void webserver::sendError(int fd, const char* info){
    assert(fd >= 0);
    //BUG:  注意这里sizeof(info)是8字节指针大小
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);
}

int webserver::setFdNonBlock(int fd){
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    return fcntl(fd, F_SETFL, new_opt);
}