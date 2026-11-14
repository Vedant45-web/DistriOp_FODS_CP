#include "json_utils.hpp"
#include "fods.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <regex>
#include <cmath>
#include <algorithm>
#include <cstdio>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
using sock_t=SOCKET;
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/sysinfo.h>
#include <unistd.h>
using sock_t=int;
#endif
static void closeSock(sock_t s){
#ifdef _WIN32
 closesocket(s);
#else
 close(s);
#endif
}
static std::string token(){std::mt19937_64 r(std::random_device{}());std::ostringstream o;o<<std::hex<<r()<<r();return o.str();}
static std::string jsonField(const std::string&s,const std::string&k){std::regex re("\\\""+k+"\\\"\\s*:\\s*\\\"([^\\\"]*)\\\"");std::smatch m;return std::regex_search(s,m,re)?m[1].str():"";}
static int numberField(const std::string&s,const std::string&k,int d=0){std::regex re("\\\""+k+"\\\"\\s*:\\s*(-?[0-9]+)");std::smatch m;return std::regex_search(s,m,re)?std::stoi(m[1].str()):d;}
static std::string post(const std::string&ip,int port,const std::string&path,const std::string&body){
#ifdef _WIN32
 WSADATA w{};WSAStartup(MAKEWORD(2,2),&w);
#endif
 sock_t s=socket(AF_INET,SOCK_STREAM,0);
#ifdef _WIN32
 if(s==INVALID_SOCKET)return "";
#else
 if(s<0)return "";
#endif
 sockaddr_in a{};a.sin_family=AF_INET;a.sin_port=htons((unsigned short)port);if(inet_pton(AF_INET,ip.c_str(),&a.sin_addr)!=1){closeSock(s);return "";}if(connect(s,(sockaddr*)&a,sizeof(a))<0){closeSock(s);return "";}
 std::ostringstream req;req<<"POST "<<path<<" HTTP/1.1\r\nHost: "<<ip<<"\r\nContent-Type: application/json\r\nContent-Length: "<<body.size()<<"\r\nConnection: close\r\n\r\n"<<body;auto data=req.str();send(s,data.c_str(),(int)data.size(),0);std::string response;char b[4096];int n;while((n=recv(s,b,sizeof(b),0))>0)response.append(b,n);closeSock(s);auto p=response.find("\r\n\r\n");return p==std::string::npos?"":response.substr(p+4);
}
static int cpuCores(){
#ifdef _WIN32
 SYSTEM_INFO s{};GetSystemInfo(&s);return (int)s.dwNumberOfProcessors;
#else
 long n=sysconf(_SC_NPROCESSORS_ONLN);return n>0?(int)n:1;
#endif
}
static double ramGB(){
#ifdef _WIN32
 MEMORYSTATUSEX s{};s.dwLength=sizeof(s);if(GlobalMemoryStatusEx(&s))return (double)s.ullTotalPhys/(1024.0*1024*1024);
#else
 struct sysinfo s{};if(sysinfo(&s)==0)return (double)s.totalram*s.mem_unit/(1024.0*1024*1024);
#endif
 return 4.0;
}
static std::pair<std::string,double> gpuInfo(){
#ifdef _WIN32
 FILE*f=_popen("nvidia-smi --query-gpu=name,memory.total --format=csv,noheader,nounits 2>NUL","r");
#else
 FILE*f=popen("nvidia-smi --query-gpu=name,memory.total --format=csv,noheader,nounits 2>/dev/null","r");
#endif
 if(!f) return {"No CUDA NVIDIA GPU detected",0};
 char b[512]{}; std::string line; if(fgets(b,sizeof(b),f)) line=b;
#ifdef _WIN32
 _pclose(f);
#else
 pclose(f);
#endif
 if(line.empty()) return {"No CUDA NVIDIA GPU detected",0};
 auto comma=line.find(','); if(comma==std::string::npos) return {line,0};
 std::string name=line.substr(0,comma); double mib=0; try{mib=std::stod(line.substr(comma+1));}catch(...){} return {name,mib/1024.0};
}
static std::string runWork(const std::string&type,int units){using clock=std::chrono::steady_clock;auto start=clock::now();units=std::max(100,std::min(units,2000000));long long checksum=0;
 if(type=="Matrix Computation"){int n=std::max(8,std::min(70,(int)std::sqrt(units/3.0)));std::vector<double>a(n*n),b(n*n),c(n*n);for(int i=0;i<n*n;i++){a[i]=(i%13)*.1;b[i]=(i%7)*.2;}for(int i=0;i<n;i++)for(int k=0;k<n;k++)for(int j=0;j<n;j++)c[i*n+j]+=a[i*n+k]*b[k*n+j];checksum=(long long)(c[n*n-1]*1000);}
 else if(type=="AI Inference"){const int width=64;std::vector<double>input(width),hidden(width),output(width);for(int i=0;i<width;i++)input[i]=(i%9)*.05;int passes=std::max(1,std::min(2000,units/1000));for(int p=0;p<passes;p++){for(int j=0;j<width;j++){double sum=0;for(int i=0;i<width;i++)sum+=input[i]*(((i*17+j*13)%23)*.01);hidden[j]=std::max(0.0,sum);}for(int j=0;j<width;j++){double sum=0;for(int i=0;i<width;i++)sum+=hidden[i]*(((i*7+j*19)%29)*.01);output[j]=1.0/(1.0+std::exp(-sum));}input.swap(output);}checksum=(long long)(input[0]*1000000);}
 else if(type=="Data Processing"){std::vector<int>v(units);for(int i=0;i<units;i++)v[i]=(i*7919)%100003;mergeSort(v);checksum=v.front()+v.back();}
 else if(type=="Image Processing"){int width=std::max(10,(int)std::sqrt(units));int height=std::max(10,units/width);std::vector<unsigned char>pixels((size_t)width*height);for(size_t i=0;i<pixels.size();++i)pixels[i]=(unsigned char)((i*31)%256);long long sum=0;for(int y=1;y<height-1;y++)for(int x=1;x<width-1;x++){int idx=y*width+x;int value=pixels[idx-1]+pixels[idx+1]+pixels[idx-width]+pixels[idx+width];sum+=value/4;}checksum=sum;}
 else {int n=std::max(100,units/10);for(int x=2;x<n;x++){bool prime=true;for(int d=2;d*d<=x;d++)if(x%d==0){prime=false;break;}if(prime)checksum++;}}
 double sec=std::chrono::duration<double>(clock::now()-start).count();std::ostringstream o;o<<"Workload executed locally; checksum="<<checksum<<"; elapsed="<<sec<<" s";return o.str();}
int main(int argc,char**argv){if(argc<3){std::cout<<"Usage: distriopt_worker <controller-ipv4> <port> [worker-name]\nExample: distriopt_worker 192.168.1.10 8080 Lenovo-LOQ\n";return 2;}std::string ip=argv[1];int port=std::stoi(argv[2]);std::string name=argc>3?argv[3]:"DistriOpt Worker";auto gpu=gpuInfo();std::string secret=token();std::ostringstream reg;reg<<"{\"name\":\""<<jsonEscape(name)<<"\",\"cpuCores\":"<<cpuCores()<<",\"ramGB\":"<<ramGB()<<",\"gpuModel\":\""<<jsonEscape(gpu.first)<<"\",\"gpuMemoryGB\":"<<gpu.second<<",\"token\":\""<<secret<<"\"}";std::cout<<"Connecting to controller "<<ip<<":"<<port<<" ...\n";auto response=post(ip,port,"/api/workers/register",reg.str());auto workerId=jsonField(response,"workerId");if(workerId.empty()){std::cerr<<"Registration failed. Check controller IP, port, Wi-Fi and firewall.\n";return 1;}std::cout<<"Registered as "<<workerId<<". Hardware: "<<cpuCores()<<" CPU cores, "<<ramGB()<<" GB RAM, GPU: "<<gpu.first<<" ("<<gpu.second<<" GB).\nPress Ctrl+C to disconnect.\n";
 while(true){std::ostringstream hb;hb<<"{\"workerId\":\""<<workerId<<"\",\"token\":\""<<secret<<"\"}";post(ip,port,"/api/workers/heartbeat",hb.str());auto claim=post(ip,port,"/api/workers/claim",hb.str());auto jobPos=claim.find("\"job\":{");if(jobPos!=std::string::npos){auto obj=claim.substr(jobPos);auto id=jsonField(obj,"id"),type=jsonField(obj,"type");int units=numberField(obj,"workUnits",10000);if(!id.empty()){auto t0=std::chrono::steady_clock::now();std::string result=runWork(type,units);double seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-t0).count();std::ostringstream out;out<<"{\"workerId\":\""<<workerId<<"\",\"token\":\""<<secret<<"\",\"jobId\":\""<<id<<"\",\"success\":1,\"seconds\":"<<seconds<<",\"result\":\""<<jsonEscape(result)<<"\"}";post(ip,port,"/api/workers/result",out.str());std::cout<<"Completed "<<id<<" in "<<seconds<<" seconds\n";}}std::this_thread::sleep_for(std::chrono::seconds(2));}
}
