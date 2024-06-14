#include "httpresponse.h"
using std::unordered_map;
using std::string;
using std::to_string;
const unordered_map<string, string> http_response::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

const unordered_map<int, string> http_response::CODE_STATUS = {
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

const unordered_map<int, string> http_response::CODE_PATH = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};
http_response::http_response(){
    _src_dir = _path = "";
    _code = -1;
    _is_keep_alive = false;
    mm_file = nullptr;
    mm_file_stat = {0};
}

http_response::~http_response(){
    if (mm_file){
        munmap(mm_file, mm_file_stat.st_size);
        mm_file = nullptr;
    }
}
void http_response::init(const std::string& src_dir, const std::string& path, 
                        bool is_keep_alive, int code){
    _src_dir = src_dir;
    _path = path;
    _is_keep_alive = is_keep_alive;
    _code = code;
    /*NOTE: 一个response实例可能复用？所以下面需处理*/
    if (mm_file)
        munmap(mm_file, mm_file_stat.st_size);
    mm_file = nullptr;
    mm_file_stat = {0};
}

void http_response::makeResponse(buffer& buff){
    if (stat((_src_dir + _path).data(), &mm_file_stat) < 0)
        _code = 404;
    /*NOTE: 如果路径对应的是一个目录*/
    else if (S_ISDIR(mm_file_stat.st_mode))
        _code = 404;
    /*NOTE: 如果不可读, 禁止访问*/
    else if (!(mm_file_stat.st_mode & S_IROTH))
        _code = 403;
    else
        _code = 200;
    errorHtml();
    addStateLine(buff);
    addHeader(buff);
    addContent(buff);
}

void http_response::addErrorContent(buffer& buff, const string& message){
    string body;
    string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if (CODE_STATUS.count(_code) == 1)
        status = CODE_STATUS.find(_code)->second;
    else
        status = "Bad Request";
    body += to_string(_code) + " : " + status + '\n';
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buff.append("Content-length: " + to_string(body.size()) + "\r\n\r\n");
    buff.append(body);
}
void http_response::addStateLine(buffer& buff){
    string status;
    if (!CODE_STATUS.count(_code))
        _code = 400;
    status = CODE_STATUS.find(_code)->second;
    buff.append("HTTP/1.1 " + to_string(_code) + " " + status + "\r\n");
}

void http_response::addHeader(buffer& buff){
    buff.append("Connection: ");
    if (_is_keep_alive){
        buff.append("keep-alive\r\n");
        buff.append("keep-alive: max=6, timeout=120\r\n");
    }
    else{
        buff.append("close\r\n");
    }
    buff.append("Content-type: " + getFileType() + "\r\n");
}

void http_response::addContent(buffer& buff){
    int src_fd = open((_src_dir+_path).data(), O_RDONLY);
    if (src_fd < 0){
        addErrorContent(buff, "File Not Found!");
        return;
    }
    LOG_DEBUG("file path %s", (_src_dir + _path).data())
    void* mm_ret = mmap(0, mm_file_stat.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
    if (mm_ret == MAP_FAILED){
        addErrorContent(buff, "File Open Failed!");
    }
    mm_file = static_cast<char*>(mm_ret);
    close(src_fd);
    /*NOTE: 这里有个空行, 所以是两个\r\n*/
    buff.append("Content-length: " + to_string(mm_file_stat.st_size) + "\r\n\r\n");
}

void http_response::errorHtml(){
    if (CODE_PATH.count(_code) == 1){
        _path = CODE_PATH.find(_code)->second;
        stat((_src_dir + _path).data(), &mm_file_stat);
    }
}
string http_response::getFileType(){
    string::size_type idx = _path.find_last_of('.');
    if (idx == string::npos)
        return "text/plain";
    string suffix = _path.substr(idx);
    if (SUFFIX_TYPE.count(suffix) == 1)
        return SUFFIX_TYPE.find(suffix)->second;
    return "text/plain";
}