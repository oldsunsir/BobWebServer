/*TODO: http request 解析*/
#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <regex>
#include "../log/log.h"
#include "../buffer/buffer.h"
#include "../pool/sqlconnpool.h"
class http_request{
public:
    enum PARSE_STATE{
        REQUEST_LINE = 0,
        HEADERS,
        BODY,
        FINISH,
    };

    enum HTTP_CODE{
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };
    http_request() {init();}
    ~http_request() = default;

    void init();

    std::string path() const;
    std::string method() const;
    std::string version() const;
    std::string getPost(const std::string& key) const;
    
    bool isKeepAlive() const;
    bool parse(buffer& buff);

private:
    bool parseRequestLine(const std::string& line);
    void parseHeader(const std::string& line);
    void parseBody(const std::string& line);

    void parsePath();
    void parsePost();
    void parseFromUrlEncoded();
    static bool userVerify(const std::string& name, const std::string& pwd, bool isLogin);

    PARSE_STATE state;
    std::string _method, _path, _version, _body;
    std::unordered_map<std::string, std::string> header;
    std::unordered_map<std::string, std::string> post;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;

};

#endif