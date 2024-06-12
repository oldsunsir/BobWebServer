/*TODO: 通过vector封装char, 实现自动增长的缓冲区*/

#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <cstddef>  //size_t
#include <string>
#include <cstring>  //bzero
#include <assert.h>
#include <sys/uio.h>    //readv
#include <unistd.h> //write
class buffer{
public:
    buffer(size_t init_buffer_size = 1024);
    ~buffer() = default;

    size_t writablBytes() const;
    size_t readableBytes() const;
    /*NOTE: prependable: 表示已经有多少个读取完毕, 这些空间可以直接拿来重新用*/
    size_t prependableBytes() const;

    /*返回指向缓冲区当前读取位置的指针*/
    const char* peek() const;
    void ensureWritable(size_t len);
    void hasWritten(size_t len);

    /*retrieve调整读取位置*/
    void retrieve(size_t len);
    void retrieveUntil(const char* end);
    void retrieveAll();
    std::string retrieveAllToStr();

    char* beginWrite();
    const char* beginWrite() const;

    void append(const char* str, size_t len);
    void append(const std::string& str);
    void append(const void* data, size_t len);
    void append(const buffer& buff);

    ssize_t readFd(int fd, int* _errno);
    ssize_t writeFd(int fd, int* _errno);
private:
    char* beginPtr();
    const char* beginPtr() const;
    void makeSpace(size_t len);

    std::vector<char> _buffer;
    size_t read_pos;
    size_t write_pos;
};
#endif