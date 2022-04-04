#pragma once 


//已经存在套接字，线程通过套接字处理任务

#include<iostream>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include"Util.h"
#include<string>
#include<vector>
#include"Log.h"

//HTTP响应报文
class HttpResponse{
  public:
    std::string StatusLine_HTTP;//状态行
    std::vector<std::string>ResponHeads;//首部字段
    std::string ResponBlank;//空行
    std::string ResponBody;//正文
};

//HTTP请求报文
class HttpRequest{
  public:
    std::string RequestLine_HTTP;//请求行
    std::vector<std::string>RequestHeads;//首部字段
    std::string RequestBlank;//空行
    std::string RequestBody;//正文
};


//读取请求，分析请求，构建响应，基本IO通信，实现基本业务逻辑
class EndPoint{
  private:
    int sock;
    HttpRequest http_request;//http请求
    HttpResponse http_response;//http响应
  private:
    void GetHttpRequestLine(){
      Util::ReadLine(sock,http_request.RequestLine_HTTP);//读取HTTP请求第一行
    }
  public:
    EndPoint(int _sock):sock(_sock){}

    void RecvQuest_HTTP(){//读取请求

    }

    void AnalyQuest_HTTP(){//解析请求
    }

    void MakeRespon_HTTP(){//构建响应

    }

    void SendRespon_HTTP(){//发送响应

    }

    ~EndPoint(){}
};

class Entry{//线程执行任务的入口
  public:
    static void*SolveQuest(void*_sock){
      ERRORLOG(INFO,"Processing Requests...");
      int sock=*(int*)_sock;
      delete(int*)_sock;
      //std::cout<<" Get a New Link: sock="<<sock<<std::endl;
      EndPoint* endpoint=new EndPoint(sock);
      endpoint->RecvQuest_HTTP();
      endpoint->AnalyQuest_HTTP();
      endpoint->MakeRespon_HTTP();
      endpoint->SendRespon_HTTP();
      delete endpoint;
      ERRORLOG(INFO,"Processing Request End!");
      return nullptr;
    }
};
