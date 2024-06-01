/*TODO: 完成同步+异步日志模块
        单例懒汉模式获得实例
        
        共有的获取实例方法
        初始化日志
        异步日志写入方法
        内容格式化方法
        刷新缓冲区*/
#ifndef LOG_H
#define LOG_H
#include <mutex>
#include <thread>
#include <string.h>
#include <sys/stat.h>   //for mkdir
#include <sys/time.h>
#include <stdarg.h>
#include <fstream>
#include "blockqueue.h"

class log{
public:
    void init(const char* path, const char* suffix = ".log",
                int max_queue_capacity = 1024, const int max_lines = 50000);
    static log* getInstance();
    static void flushLogThread();

    void write_log(int level, const char* format, ...);
    void flush();

    bool isOpen(){
        return is_open;
    }
private:
    log();
    /*NOTE: 这里将析构设置为private, 防止手动删除
            而静态变量生命周期与进程一致, 进程结束后
            会自动调用析构释放, 所以析构是有用的    */
    virtual ~log();
    log& operator= (const log&) = delete;
    log(const long&) = delete;

    void clear_deque_to_file();
    void asyncWrite();

    const char* dir_name;   //这里是指向const char的指针, 不是常量成员
    const char* _suffix;
    
    static const int MAX_BUF_SIZE = 8192;

    int _max_lines;
    int line_cnt;
    int today;

    bool is_open;
    bool is_async;
    char* buf;

    std::ofstream ofs;
    std::unique_ptr<block_queue<std::string>> deque;
    std::unique_ptr<std::thread> write_thread;
    std::mutex mtx;
};

/*BUG:  这里不要用一个临时变量, 如
        log* my_log = log::getInstance(),
        会报错重定义    */

/*NOTE: 这里的flush, 虽然跟在write_log后, 但刷新的
        不一定是这条语句, 消费者线程更慢一点, 所以flush的时候
        本语句还未进入ofs   */
#define LOG_BASE(level, format, ...) \
        if (log::getInstance()->isOpen()){\
            log::getInstance()->write_log(level, format, ##__VA_ARGS__);\
            log::getInstance()->flush();\
        }\
    
#define LOG_DEBUG(format, ...)  LOG_BASE(0, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...)   LOG_BASE(1, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)   LOG_BASE(2, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...)  LOG_BASE(3, format, ##__VA_ARGS__)

#endif