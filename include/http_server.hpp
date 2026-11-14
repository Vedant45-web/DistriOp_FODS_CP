#pragma once
#include <string>
#include <functional>
#include <map>
struct HttpRequest { std::string method,path,body; };
struct HttpResponse { int status{200}; std::string contentType{"application/json"},body; };
class HttpServer {
public:
    using Handler=std::function<HttpResponse(const HttpRequest&)>;
    void get(const std::string& path,Handler h); void post(const std::string& path,Handler h);
    bool listen(unsigned short port);
private:
    std::map<std::string,Handler> get_,post_;
    HttpResponse dispatch(const HttpRequest& req);
};
