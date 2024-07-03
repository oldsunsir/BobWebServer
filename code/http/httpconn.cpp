#include "httpconn.h"

/*BUG:  这里必须定义, 静态变量在类中只是声明, 需要定义分配内存, 再在其他地方初始化或赋值*/
bool http_conn::is_et = true;
const char* http_conn::src_dir = nullptr;
std::atomic<int> http_conn::user_cnt{0};


http_conn::http_conn(){
    _fd = -1;
    _addr = {0};
    _is_close = true;
    iov_cnt = 0;
    bzero(iov, sizeof(iov));
}

void http_conn::init(int fd, const sockaddr_in& addr){
    _fd = fd;
    _addr = addr;
    user_cnt++;
    read_buff.retrieveAll();
    write_buff.retrieveAll();
    _is_close = false;
    /*BUG:  atomic不可以直接拷贝*/
    LOG_INFO("client[%d](%s:%d) in, user_cnt:%d", _fd, getIP(), getPort(), static_cast<int>(user_cnt))
}

http_conn::~http_conn(){
    LOG_INFO("conn[%d] 在这里析构", _fd)
    close();
}

void http_conn::close(){
    /*NOTE: 记着把http_response中的unmap*/
    _response.unmapFile();
    if (!_is_close){
        _is_close = true;
        user_cnt--;
        ::close(_fd);
        // LOG_INFO("client[%d](%s:%d) quit, user_cnt:%d", _fd, getIP(), getPort(), static_cast<int>(user_cnt));
    }
}

int http_conn::getFd() const{
    return _fd;
}
int http_conn::getPort() const{
    return _addr.sin_port;
}
char* http_conn::getIP() const{
    return inet_ntoa(_addr.sin_addr);
}
sockaddr_in http_conn::getAddr() const{
    return _addr;
}

ssize_t http_conn::read(int* save_errno){
    ssize_t len = -1;
    /*BUG:  如果启用et循环读, _fd一定要设置非阻塞, 否则会在这里阻塞*/
    do {
        len = read_buff.readFd(_fd, save_errno);
        if (len <= 0)
            break;
    }   while (is_et);
    return len;
}

bool http_conn::process(){
    _request.init();
    if (read_buff.readableBytes() <= 0)
        return false;

    if (_request.parse(read_buff)){
        _response.init(src_dir, _request.path(), _request.isKeepAlive());
    }
    else{
        _response.init(src_dir, "", false);
    }
    _response.makeResponse(write_buff);
    iov[0].iov_base = const_cast<char*> (write_buff.peek());
    iov[0].iov_len = write_buff.readableBytes();
    /*BUG:  这里不能用iov_cnt++
            因为一个连接是会复用的, 那么第二次process的时候
            iov_cnt岂不是大于2了吗, 又在这个地方卡了好久。。。*/
    iov_cnt = 1;
    if (_response.file() != nullptr){
        iov[1].iov_base = _response.file();
        iov[1].iov_len = _response.fileLen();
        iov_cnt = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", _response.fileLen() , iov_cnt, iov[0].iov_len + iov[1].iov_len);
    return true;
}

ssize_t http_conn::write(int* save_errno){
    ssize_t len = -1;
    do {
        len = writev(_fd, iov, iov_cnt);
        if (len <= 0){
            *save_errno = errno;
            break;
        }
        /*NOTE: 传输完*/
        if (len >= iov[0].iov_len + iov[1].iov_len){
            write_buff.retrieveAll();
            /*BUG:  这里记得处理长度, 因为后续需要通过toWriteBytes来判断是否需要继续写*/
            iov[0].iov_len = 0;
            iov[1].iov_len = 0;
            break;
        }
        else if (len >= iov[0].iov_len){
            iov[1].iov_base = (uint8_t*)iov[1].iov_base + len - iov[0].iov_len;
            iov[1].iov_len -= len - iov[0].iov_len;
            if (iov[0].iov_len){
                iov[0].iov_len = 0;
                write_buff.retrieveAll();
            }
        }
        else{
            iov[0].iov_base = (uint8_t*)iov[0].iov_base + len;
            iov[0].iov_len -= len;
            write_buff.retrieve(len);
        }
        /*NOTE: 为什么原作者这里加上对ToWriteBytes > 10240的判断？*/
    }   while (is_et);
    return len;
}