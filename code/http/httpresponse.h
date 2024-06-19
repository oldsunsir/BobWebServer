/*TODO: 生成http的相应*/
#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H
#include "../log/log.h"
#include "../buffer/buffer.h"
#include <sys/stat.h>   //stat
#include <sys/mman.h>
#include <unordered_map>
#include <string>
#include <unistd.h>
#include <fcntl.h>
class http_response{
public:
    http_response();
    ~http_response();
    void init(const std::string& src_dir, const std::string& path,
                bool is_keep_alive = false, int code = -1);
    void makeResponse(buffer& buff);
    /*NOTE: 用来close连接时取消映射*/
    void unmapFile();

    void addErrorContent(buffer& buff, const std::string& message);
    char* file() const{
        return mm_file;
    }
    size_t fileLen() const{
        return mm_file_stat.st_size;
    }
    int code() const{
        return _code;
    }
private:
    void addStateLine(buffer& buff);
    void addHeader(buffer& buff);
    void addContent(buffer& buff);

    /*NOTE: 如果访问出错, 统一用errorHtml处理, 转到访问错误页面的html*/
    void errorHtml();
    /*INFO: http需要用到MIME的文件类型*/
    std::string getFileType();

    std::string _src_dir;
    std::string _path;

    int _code;
    bool _is_keep_alive;
    /*NOTE: 考虑到消息主体可能较大, 将文件映射到内存提高访问速度*/
    char* mm_file;
    /*NOTE: stat获取文件状态信息, 如长度等*/
    struct stat mm_file_stat;
    /*NOTE: 用于将文件后缀映射到相应的MIME类型*/
    /*INFO: MIME:https://www.runoob.com/http/mime-types.html*/
    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
};

#endif