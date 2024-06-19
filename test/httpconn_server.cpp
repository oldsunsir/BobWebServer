#include "../code/http/httpconn.h"  
#include <netinet/in.h>
#include <assert.h>


// bool http_conn::is_et = true;
// const char* http_conn::src_dir = "../../resources";
// std::atomic<int> http_conn::user_cnt{0};

int main(int argc, char **argv) {
    log::getInstance()->init("./log", ".http_conn_test_log", 1024);
    sql_pool::instance()->init("localhost", 3306, "test_user", "test_password", "test_db", 5);

    sockaddr_in address;
    sockaddr_in client_address;
    http_conn test_conn;
    int save_errno;

    bzero(&address, 0);
    bzero(&client_address, 0);
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);

    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listen_fd >= 0);

    int ret = bind(listen_fd, (struct sockaddr*)(&address), sizeof(address));
    assert(ret >= 0);

    ret = listen(listen_fd, 5);
    assert(ret >= 0);

    socklen_t addr_len = sizeof(address);
    int conn_fd = accept(listen_fd, (struct sockaddr*)(&client_address), &addr_len);
    assert(conn_fd >= 0);
    int old_flags = fcntl(conn_fd, F_GETFL);
    int new_flags = old_flags | O_NONBLOCK;
    fcntl(conn_fd, F_SETFL, new_flags);

    test_conn.init(conn_fd, client_address);
    /*BUG:  建议这里为了确保能读到数据, 使用阻塞fd或者非阻塞模式+select/poll/epoll
            因为有时候会出现即使发送了ack之后, len = read_buff.readFd(_fd, save_errno) 这里
            非阻塞读返回-1, 为什么呢？？？*/

    /*INFO: 分析:    在et为true, 且采用非阻塞fd的情况下, 有时候会出现read不到数据的问题
                    可能因为我这里客户端发送数据和服务端接收数据谁先进行的问题
                    假如accept后服务端直接read了, 这时客户端还未传送来数据, 那由于我是非阻塞fd
                    read会返回-1, 数据也就没进入readbuff中, process返回false, 然后服务端
                    向客户端发送RST
                    
                    所以这里sleep()的时间比客户端的久, 应该可以保证客户端先发数据过来吧*/
    sleep(4);
    test_conn.read(&save_errno);
    assert(test_conn.process());
    test_conn.write(&save_errno);

    test_conn.close();
    close(listen_fd);
}