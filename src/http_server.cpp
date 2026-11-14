#include "http_server.hpp"
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <fstream>
#include <iterator>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#define CLOSESOCK closesocket
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
typedef int socket_t;
#define CLOSESOCK close
#endif
void HttpServer::get(const std::string&p,Handler h){get_[p]=std::move(h);} void HttpServer::post(const std::string&p,Handler h){post_[p]=std::move(h);}
HttpResponse HttpServer::dispatch(const HttpRequest&r){
    if (r.method == "OPTIONS") {
        return {200, "text/plain", ""};
    }
    auto& routes=r.method=="POST"?post_:get_;
    auto it=routes.find(r.path);
    if(it!=routes.end())return it->second(r);
    return {404,"application/json","{\"error\":\"Not found\"}"};
}
bool HttpServer::listen(unsigned short port){
#ifdef _WIN32
 WSADATA wsa{}; if(WSAStartup(MAKEWORD(2,2),&wsa)!=0){std::cerr<<"WSAStartup failed\n"<<std::flush;return false;}
#endif
 socket_t server=::socket(AF_INET,SOCK_STREAM,0);
#ifdef _WIN32
 if(server==INVALID_SOCKET){
#else
 if(server<0){
#endif
 std::cerr<<"Socket creation failed\n"<<std::flush;return false;}
 int opt=1;setsockopt(server,SOL_SOCKET,SO_REUSEADDR,reinterpret_cast<const char*>(&opt),sizeof(opt));sockaddr_in addr{};addr.sin_family=AF_INET;addr.sin_addr.s_addr=INADDR_ANY;addr.sin_port=htons(port);if(::bind(server,reinterpret_cast<sockaddr*>(&addr),sizeof(addr))<0){std::cerr<<"Bind failed on port "<<port<<". Is port "<<port<<" already in use?\n"<<std::flush;CLOSESOCK(server);return false;}if(::listen(server,16)<0){std::cerr<<"Listen failed\n"<<std::flush;CLOSESOCK(server);return false;}
 std::cout<<"DistriOpt server listening on http://localhost:"<<port<<"\n"<<std::flush;
 while(true){sockaddr_in clientAddr{};
#ifdef _WIN32
 int len=sizeof(clientAddr);
#else
 socklen_t len=sizeof(clientAddr);
#endif
 socket_t client=::accept(server,reinterpret_cast<sockaddr*>(&clientAddr),&len);
#ifdef _WIN32
 if(client==INVALID_SOCKET)continue;
#else
 if(client<0)continue;
#endif
 std::thread([this,client](){std::string data;char buf[4096];size_t headerEnd=std::string::npos;size_t contentLength=0;while(data.size()<1024*1024){int n=recv(client,buf,sizeof(buf),0);if(n<=0)break;data.append(buf,n);if(headerEnd==std::string::npos){headerEnd=data.find("\r\n\r\n");if(headerEnd!=std::string::npos){std::string headers=data.substr(0,headerEnd);auto p=headers.find("Content-Length:");if(p==std::string::npos)p=headers.find("content-length:");if(p!=std::string::npos){p=headers.find(':',p)+1;while(p<headers.size()&&headers[p]==' ')p++;try{contentLength=std::stoul(headers.substr(p));}catch(...){contentLength=0;}}}}if(headerEnd!=std::string::npos&&data.size()>=headerEnd+4+contentLength)break;}
 HttpRequest req;auto lineEnd=data.find("\r\n");if(lineEnd!=std::string::npos){std::istringstream line(data.substr(0,lineEnd));line>>req.method>>req.path;auto q=req.path.find('?');if(q!=std::string::npos)req.path=req.path.substr(0,q);if(headerEnd!=std::string::npos)req.body=data.substr(headerEnd+4,contentLength);}
 HttpResponse res=dispatch(req);std::ostringstream out;std::string status="OK";
 if(res.status==200) status="OK";
 else if(res.status==204) status="No Content";
 else if(res.status==400) status="Bad Request";
 else if(res.status==401) status="Unauthorized";
 else if(res.status==404) status="Not Found";
 else if(res.status==409) status="Conflict";
 else status="Internal Server Error";
 out<<"HTTP/1.1 "<<res.status<<" "<<status<<"\r\nContent-Type: "<<res.contentType<<"; charset=utf-8\r\nContent-Length: "<<res.body.size()<<"\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With\r\nAccess-Control-Allow-Methods: GET, POST, OPTIONS\r\nConnection: close\r\n\r\n"<<res.body;std::string response=out.str();send(client,response.c_str(),static_cast<int>(response.size()),0);CLOSESOCK(client);}).detach();
 }
 CLOSESOCK(server);return true;
}
