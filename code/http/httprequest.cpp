#include "httprequest.h"

using std::unordered_map;
using std::unordered_set;
using std::string;

static int converHex(char ch){
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return -1;
}
const unordered_set<string> http_request::DEFAULT_HTML{
    "/index", "/register", "/login",
    "/welcome", "/video", "/picture",
};

const unordered_map<string, int> http_request::DEFAULT_HTML_TAG{
    {"/register.html", 0}, {"/login.html", 1},
};

void http_request::init(){
    _method, _path, _version, _body = "";
    state = REQUEST_LINE;
    header.clear();
    post.clear();
}

string http_request::path() const{
    return _path;
}

string http_request::method() const{
    return _method;
}

string http_request::version() const{
    return _version;
}

string http_request::getPost(const std::string& key) const{
    if (post.count(key) == 1)
        return post.find(key)->second;
    return "";
}

bool http_request::isKeepAlive() const{
    if (header.count("Connection") == 1){
        return header.find("Connection")->second == "keep-alive"
            && _version == "1.1";
    }
    return false;
}
bool http_request::parse(buffer& buff){
    const char CRLF[] = "\r\n";
    if (buff.readableBytes() <= 0)
        return false;
    while (buff.readableBytes() && state != FINISH){
        /*BUG:  这里需要手动添加上const属性 */
        const char* line_end = std::search(buff.peek(), const_cast<const char*>(buff.beginWrite()), CRLF, CRLF+2);
        string line(buff.peek(), line_end);
        LOG_DEBUG("state : %d, line : %s", state, line.c_str())
        switch (state){
            case REQUEST_LINE:
                if (!parseRequestLine(line)){
                    return false;
                }
                parsePath();
                break;
            case HEADERS:
                // LOG_INFO("file : %s, line : %s, readableBytes : %d", __FILE__, __LINE__, buff.readableBytes())
                parseHeader(line);
                /*NOTE: 如果空行后没有请求体, 直接完成*/ 
                if (buff.readableBytes() <= 2)
                    state = FINISH;
                break;
            case BODY:
                LOG_DEBUG("当前为body, line : %s", line.c_str())
                parseBody(line);
                break;
            default:
                break;
        }
        /*NOTE: 如果未找到CRLF*/
        /*BUG:  POST中请求体是不一定以\r\n结尾的, 所以直接只通过
                line_end == buff.beginWrite() 来判断是不对的*/
        if (line_end == buff.beginWrite() && (state != FINISH)){
            LOG_ERROR("HTTP REQUEST PARSE ERROR!")
            return false;
            break;
        }
        else if (line_end == buff.beginWrite()){
            buff.retrieveAll();
        }
        else{
            buff.retrieveUntil(line_end+2);
        }
    }
    LOG_DEBUG("[%s], [%s], [%s]", _method.c_str(), _path.c_str(), _version.c_str())
    return true;
}
bool http_request::parseRequestLine(const string& line){
    /*INFO: [^ ]表示匹配非空格的字符, * 表示匹配一个或多个
            开头^匹配字符串的开头, $匹配字符串末尾  */
    std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch sub_match;
    if (std::regex_match(line, sub_match, pattern)){
        _method = sub_match[1];
        _path = sub_match[2];
        _version = sub_match[3];
        state = HEADERS;
        return true;
    }
    LOG_ERROR("RequestLine Error")
    return false;
}

void http_request::parseHeader(const string& line){
    std::regex pattern("^([^:]*): ?(.*)$");
    std::smatch sub_match;
    if (std::regex_match(line, sub_match, pattern)){
        header[sub_match[1]] = sub_match[2];
    }
    else{
        /*NOTE: 没找到表示当前是空行, 接下来该处理body*/
        state = BODY;
    }
}

void http_request::parseBody(const std::string& line){
    _body = line;
    /*TODO: 这里只处理了GET和POST两种请求*/
    parsePost();
    state = FINISH;
    LOG_DEBUG("file : %s,len:%d", __FILE__, line.size())
}

void http_request::parsePath(){
    if (_path == "/")
        _path = "/index.html";
    else{
        for (auto& item : DEFAULT_HTML){
            if (item == _path){
                _path += ".html";
                break;
            }
        }
    }
}

void http_request::parsePost(){
    /*NOTE: Post默认编码格式为application/x-www-form-urlencoded */
    /*INFO: 键值对用=链接   多个键值对&分割 特殊字符%后跟两个十六进制表示   空格用+代替*/
    if (_method == "POST" && header["Content-Type"] == "application/x-www-form-urlencoded"){
        parseFromUrlEncoded();
        if (DEFAULT_HTML_TAG.count(_path)){
            int tag = DEFAULT_HTML_TAG.find(_path)->second;
            LOG_DEBUG("Tag:%d", tag);
            bool isLogin = (tag == 1);
            if (userVerify(post["username"], post["password"], isLogin))
                _path = "/welcome.html";
            else
                _path = "/error.html";
        }
    }
}

void http_request::parseFromUrlEncoded(){
    string key = "";
    string value = "";
    int n = _body.size();
    int num = 0;
    int left = 0, right = 0;
    for (; right < n; ++right){
        char ch = _body[right];
        switch (ch){
            case '=':
                key = _body.substr(left, right-left);
                left = right + 1;
                break;
            case '+':
                _body[right] = ' ';
                break;
            case '%':
                num = converHex(_body[right+1])*16 + converHex(_body[right+2]);
                _body[right+2] = num % 10 + '0';
                _body[right+1] = num / 10 + '0';
                right += 2;
                break;
            case '&':
                value = _body.substr(left, right-left);
                left = right + 1;
                post[key] = value;
                LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
                break;
            default:
                break;
        }
    }
    assert(left <= right);
    /*NOTE: 注意最后一个key*/
    if (post.count(key) == 0 && left < right){
        value = _body.substr(left, right-left);
        post[key] = value;
    }    
}

bool http_request::userVerify(const std::string& name, const std::string& pwd, bool isLogin){
    LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str())
    MYSQL* sql = nullptr;
    //BUG:  一开始这里写法是conn_RAII (&sql, sql_pool::instance());
    /*  显然不对, 这个表达式结束就直接析构, conn返回, 这可能会导致多个用户用一个conn连接*/
    conn_RAII conn_raii(&sql, sql_pool::instance());
    assert(sql);

    bool flag = false;
    unsigned int j = 0;
    char order[256] = {0};
    MYSQL_FIELD* fields = nullptr;
    MYSQL_RES* res = nullptr;

    if (!isLogin)
        flag = true;
    
    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", name.c_str());
    LOG_DEBUG("%s", order);
    /*INFO: 为0表示执行成功, 否则返回非零值*/
    if (mysql_query(sql, order)){
        mysql_free_result(res);
        return false;
    }
    res = mysql_store_result(sql);
    
    while(MYSQL_ROW row = mysql_fetch_row(res)){
        LOG_DEBUG("MYSQL ROW : %s %s", row[0], row[1]);
        string password(row[1]);
        if (isLogin){
            if (pwd == password){
                LOG_INFO("login successfully")
                flag = true;
            }
            else{
                flag = false;
                LOG_ERROR("pwd error")
            }
        }
        else{
            /*NOTE: 有用户名但不是登录方式*/
            flag = false;
            LOG_ERROR("user used")
        }
    }
    mysql_free_result(res);

    /*注册行为*/
    if (!isLogin && flag == true){
        LOG_DEBUG("register")
        bzero(order, 256);
        snprintf(order, 256, "INSERT INTO user(username, password) VALUES('%s','%s')", name.c_str(), pwd.c_str());
        LOG_DEBUG("%s", order)
        if (mysql_query(sql, order)){
            LOG_ERROR("Insert error")
            flag = false;
            return flag;
        }
        flag = true;
    }
    LOG_DEBUG("UserVerify success")
    return flag;
}