#include "log.h"   
#include <iostream>
log::log(){
    line_cnt = 0;
    is_async = false;
    is_open = true;

    deque = nullptr;
    write_thread = nullptr;
}

log::~log(){
    /*BUG:  如果不判断直接close或join, 会有段错误, 
            因为同步情况下他们是nullptr */
    if (is_async){
        /*NOTE: 直接先close, 会导致队列里面的日志未输出,
                所以可以先把他们都flush到文件中再关闭   */
        clear_deque_to_file();
        deque->close();
        write_thread->join();
    }

    if (ofs.is_open()){
        flush();
        ofs.close();
    }
   
}
void log::clear_deque_to_file(){
    while (!deque->empty()){
        std::string left_str = "";
        deque->pop(left_str);
        ofs << left_str;
    }
}
log* log::getInstance(){
    static log inst;
    return &inst;
}

void log::flushLogThread(){
    log::getInstance()->asyncWrite();
}

void log::asyncWrite(){
    std::string str = "";
    /*  BUG:    使用!empty()作为判断
                直接退出while循环了...   
        TODO:   但我这里会空转...

        NOTE:   这里不用pop来判断, 是由于下面这种情况
                在日志刚好分页时, 如果pop了一个, 然后这时候失去锁,
                触发了clear_deque_to_file, 那这个pop的一个将无法
                按照正确顺序记录在原日志中  
                
                也就是我为了日志的完整性放弃了性能？*/
        
    while (!deque->is_close()){
        std::lock_guard<std::mutex> locker(mtx);
        if (!deque->empty()){
            deque->pop(str);
            ofs << str << std::flush;
        }
    }
}

void log::flush(){
    std::lock_guard<std::mutex> locker(mtx);
    ofs << std::flush;
}

void log::init(const char* path, const char* suffix,
                int max_queue_capacity, const int max_lines){
    if (max_queue_capacity > 0){
        is_async = true;
        deque = std::make_unique<block_queue<std::string>>(max_queue_capacity);
        write_thread = std::make_unique<std::thread>(flushLogThread);  
    }
    else
        is_async = false;
    
    buf = new char[MAX_BUF_SIZE];
    memset(buf, '\0', MAX_BUF_SIZE);

    time_t timer = time(nullptr);
    struct tm* sys_time = localtime(&timer);
    struct tm t = *sys_time;

    dir_name = path;
    _suffix = suffix;
    _max_lines = max_lines;
    char file_name[256] = {0};
    snprintf(file_name, sizeof(file_name), "%s/%04d_%02d_%02d%s",
                dir_name, t.tm_year+1900, t.tm_mon+1, t.tm_mday, _suffix);

    today = t.tm_mday;

    if (ofs.is_open()){
        flush();
        ofs.close();
    }
    ofs.open(file_name, std::ios::app);
    if (!ofs){
        mkdir(dir_name, 0777);
        ofs.open(file_name, std::ios::app);
    }

    assert(ofs.is_open());
}

void log::write_log(int level, const char* format, ...){
    struct timeval now {0, 0};
    gettimeofday(&now, nullptr);
    time_t t_sec = now.tv_sec;
    struct tm* sys_time = localtime(&t_sec);
    struct tm t = *sys_time;

    char s[16] = {0};
    switch (level){
        case 0:
            strcpy(s, "[debug]:");
            break;
        case 1:
            strcpy(s, "[info]:");
            break;
        case 2:
            strcpy(s, "[warn]:");
            break;
        case 3:
            strcpy(s, "[error]:");
            break;
        default:
            strcpy(s, "[info]:");
            break;
    }
    va_list valst;
    std::string log_str;
    va_start(valst, format);
    
    /*NOTE: 这里能保证按照时间顺序push进队列么? */
    {
        std::lock_guard<std::mutex> locker(mtx);
        line_cnt++;
        if ((today != t.tm_mday) || (line_cnt && (line_cnt % _max_lines == 0))){
            char new_file_name[256] = {0};
            char tail[36] = {0};
            snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year+1900, t.tm_mon+1, t.tm_mday);
            if (today != t.tm_mday){
                snprintf(new_file_name, sizeof(new_file_name), "%s/%s%s",
                            dir_name, tail, _suffix);
                today = t.tm_mday;
                line_cnt = 0;
            }
            else{
                snprintf(new_file_name, sizeof(new_file_name),"%s/%s-%d%s",
                            dir_name, tail, (line_cnt/_max_lines), _suffix);
            }
            clear_deque_to_file();
            ofs.flush();
            ofs.close();
            ofs.open(new_file_name, std::ios::app);
            printf("%s\n", new_file_name);
            assert(ofs.is_open());
        }
        int n = snprintf(buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                    t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                    t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec, s);
    
        int m = vsnprintf(buf+n, MAX_BUF_SIZE-n-1, format, valst);
        buf[n+m] = '\n';
        buf[n+m+1] = '\0';

        log_str = buf;

        if (is_async && deque && !deque->full()){
            deque->push(log_str);
        }
        else{
            /*NOTE: fputs是线程安全的*/
            ofs << log_str << std::flush;
            // fputs(log_str.c_str(), fp);
        }
    }
    va_end(valst);
}