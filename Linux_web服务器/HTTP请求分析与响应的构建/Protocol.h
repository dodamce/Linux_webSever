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

#include<sstream>
#include<unordered_map>

#include<sys/stat.h>
#include<algorithm>

#define OK 200
#define NOTFOUND 404

#define WEB_ROOT "wwwroot"
#define HOME_PAGE "index.html"

//HTTP响应报文
class HttpResponse{
  public:
    std::string StatusLine_HTTP;//状态行
    std::vector<std::string>ResponHeads;//首部字段
    std::string ResponBlank;//空行
    std::string ResponBody;//正文

    int status_code=OK;//响应状态码
};

//HTTP请求报文
class HttpRequest{
  public:
    std::string RequestLine_HTTP;//请求行
    std::vector<std::string>RequestHeads;//首部字段
    std::string RequestBlank;//空行
    std::string RequestBody;//正文

    //解析完请求报文后的结果
    std::string Method;
    std::string URI;//Path?Pararm
    std::string Version;

    //保存解析首部字段的map
    std::unordered_map<std::string,std::string>Head_KVS;

    int Content_Lenth=0;

    //访问资源的路径
    std::string Path;

    //如果是GET方法通过URL上传的参数
    std::string Param;

    bool CGI=false;
};


//读取请求，分析请求，构建响应，基本IO通信，实现基本业务逻辑
class EndPoint{
  private:
    int sock;
    HttpRequest http_request;//http请求
    HttpResponse http_response;//http响应
  private:
    void GetHttpRequestLine(){//读请求行
      Util::ReadLine(sock,http_request.RequestLine_HTTP);//读取HTTP请求第一行
      http_request.RequestLine_HTTP.pop_back();
      ERRORLOG(INFO,http_request.RequestLine_HTTP);
    }

    void GetHttpRequstHeads(){//读取首部字段
      std::string line;
      while(true){
        Util::ReadLine(sock,line);
        if(line=="\n"){
          ERRORLOG(INFO,line);
          http_request.RequestBlank=line;
          break;
        }
        line.pop_back();//去掉每行的\n
        http_request.RequestHeads.push_back(line);
        ERRORLOG(INFO,line);
        line.clear();
      }
    }

    void AnalyQuestLine(){//解析请求行  方法 URL HTTP版本
      std::stringstream Str(http_request.RequestLine_HTTP);
      Str>>http_request.Method>>http_request.URI>>http_request.Version;
      //将方法统一转化成大写 Get->GET
      std::string& strtmp=http_request.Method;
      std::transform(strtmp.begin(),strtmp.end(),strtmp.begin(),std::toupper);//写回strtmp首部
    }

    void AnalyuestHeadS(){
      std::string key;
      std::string value;
      for(auto&line:http_request.RequestHeads){
        if(Util::CutString(line,key,value,": ")){
          http_request.Head_KVS.insert(std::make_pair(key,value)); 

        }
        else{
          ERRORLOG(FATA,"AnalyuestHeadS error");
        }
      }
    }

    bool HaveHttpBody(){
      //判断是否是GET方法，GET方法没有正文
      std::string& Method=http_request.Method;
      if(Method=="POST"){
        std::unordered_map<std::string,std::string>::iterator iter=http_request.Head_KVS.find("Content-Lenth");
        if(iter!=http_request.Head_KVS.end()){
          http_request.Content_Lenth=atoi(iter->second.c_str());
          return true;
        }
      }
      return false;
    }

    void GetHttpBody(){
      if(HaveHttpBody()){
        int Content_Lenth=http_request.Content_Lenth;
        char ch=0;
        while(Content_Lenth>0){
          ssize_t size=recv(sock,&ch,1,0);
          if(size>0){
            http_request.RequestBody.push_back(ch);
            Content_Lenth--;
          }
          else{
            break;
          }
        }
      }
    }
  public:
    EndPoint(int _sock):sock(_sock){}

    void RecvQuest_HTTP(){//读取请求
      GetHttpRequestLine();
      GetHttpRequstHeads();
    }

    void AnalyQuest_HTTP(){//解析请求
      AnalyQuestLine();
      AnalyuestHeadS();
      GetHttpBody();
    }

    void MakeRespon_HTTP(){//构建响应
      //判断请求类型
      std::string tmpPath;
      if(http_request.Method!="GET"&&http_request.Method!="POST"){
        ERRORLOG(WARNING,"error request");
        http_response.status_code=NOTFOUND;
        goto END;
      }
      //如果是GET方法需要处理URL，看URL是否有参数
      if(http_request.Method=="GET"){
        size_t pos=http_request.URI.find('?');
        if(pos!=std::string::npos){
          Util::CutString(http_request.URI,http_request.Path,http_request.Param,"?");
          http_request.CGI=true;
        }
        else{
          //不是通过GET传参数
          http_request.Path=http_request.URI;
        }
      }
      else if(http_request.Method=="POST"){
        //POST
        http_request.CGI=true;//需要CGI技术
      }
      //std::cout<<"Debug# URI: "<<http_request.URI<<" Path:"<<http_request.Path<<" Param:"<<http_request.Param<<std::endl;
      //拼接web根目录;
      tmpPath=http_request.Path;
      http_request.Path=WEB_ROOT;
      http_request.Path+=tmpPath;
      //std::cout<<"Debug# "<<http_request.Path<<std::endl;
      if(http_request.Path[http_request.Path.size()-1]=='/'){
        //默认访问index.html
        http_request.Path+=HOME_PAGE;
      }
      //std::cout<<"Debug# "<<http_request.Path<<std::endl;
      //判断路径是否合法
      struct stat stat_buff;
      if(stat(http_request.Path.c_str(),&stat_buff)!=0){
        //资源存在，需要判断这个路径是否访问了路径下的某个资源，如果没有，直接将路径的默认网页响应回去
        if(S_ISDIR(stat_buff.st_mode)){
          //是目录
          http_request.Path+="/";
          http_request.Path+=HOME_PAGE;
        }
        if((stat_buff.st_mode &S_IXUSR)||(stat_buff.st_mode &S_IXGRP)||(stat_buff.st_mode& S_IXOTH)){
          //可执行文件，需要特殊处理
          http_request.CGI=true;
        }
      }
      else{
        //资源不存在状态码 404
        ERRORLOG(WARNING,http_request.Path+" Not Found!");
        http_response.status_code=NOTFOUND;
      }

      if(http_request.CGI==true){
        //ProceCGI();
      }
      else{
        //ProceNoCGI();//一定是GET方法，一定不带参，简单的文本网页返回
      }
END:
      return;
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
