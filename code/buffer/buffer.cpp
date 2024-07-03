#include "buffer.h"

buffer::buffer(size_t init_buffer_size) : 
    _buffer(init_buffer_size), read_pos(0), write_pos(0){}

size_t buffer::writablBytes() const{
    return _buffer.size() - write_pos;
}

size_t buffer::readableBytes() const{
    return write_pos - read_pos;
}

size_t buffer::prependableBytes() const{
    return read_pos;
}

const char* buffer::peek() const{
    return beginPtr() + read_pos;
}

void buffer::ensureWritable(size_t len){
    if (writablBytes() < len){
        makeSpace(len);
    }
    assert(writablBytes() >= len);
}

void buffer::hasWritten(size_t len){
    write_pos += len;
}

void buffer::retrieve(size_t len){
    assert(len <= readableBytes());
    read_pos += len;
}

void buffer::retrieveUntil(const char* end){
    if (peek() < end){
        if (end - peek() > readableBytes())
        //TODO:BUG:
        fprintf(stderr, "在这里出错 end - peek() = %d, readable = %d, readpos:%d", end-peek(), readableBytes(), read_pos);
    }
        retrieve(end - peek());
}
void buffer::retrieveAll(){
    bzero(&_buffer[0], _buffer.size());
    read_pos = 0;
    write_pos = 0;
}

std::string buffer::retrieveAllToStr(){
    std::string str(peek(), readableBytes());
    retrieveAll();
    return str;
}

char* buffer::beginWrite(){
    return beginPtr() + write_pos;
}

const char* buffer::beginWrite() const{
    return beginPtr() + write_pos;
}
void buffer::append(const char* str, size_t len){
    ensureWritable(len);
    std::copy(str, str+len, beginWrite());
    hasWritten(len);
}
void buffer::append(const std::string& str){
    append(str.data(), str.size());
}
void buffer::append(const void* data, size_t len){
    append(static_cast<const char*>(data), len);
}
void buffer::append(const buffer& buff){
    append(buff.peek(), buff.readableBytes());
}


char* buffer::beginPtr(){
    return &*_buffer.begin();
}

const char* buffer::beginPtr() const{
    return &*_buffer.begin();
}

void buffer::makeSpace(size_t len){
    if (writablBytes() + prependableBytes() < len)
        _buffer.resize(write_pos + len + 1);
    else{
        size_t readable = readableBytes();
        std::copy(beginPtr()+read_pos, beginPtr()+write_pos, beginPtr());
        read_pos = 0;
        write_pos = readable;
    }
}

ssize_t buffer::readFd(int fd, int* _errno){
    char buff[65535];
    /*NOTE: 分散读*/
    iovec iov[2];
    const size_t writalbe = writablBytes();
    iov[0].iov_base = beginPtr() + write_pos;
    iov[0].iov_len = writalbe;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);
    if (len < 0)
        *_errno = errno;
    else if (len <= writalbe)
        write_pos += len;
    else{
        /*NOTE: 记得先改变write_pos */
        write_pos = _buffer.size();
        append(buff, len - writalbe);
    }
    return len;
}

ssize_t buffer::writeFd(int fd, int* _errno){
    size_t readable = readableBytes();
    ssize_t len = write(fd, peek(), readable);
    if (len < 0){
        *_errno = errno;
        return len;
    }
    read_pos += len;
    return len;
}