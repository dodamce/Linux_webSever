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

#include<sys/sendfile.h>
#include<fcntl.h>

#define OK 200
#define NOTFOUND 404

#define WEB_ROOT "wwwroot"
#define HOME_PAGE "index.html"
#define VERSON_HTTP "HTTP/1.0"
#define LINE_END "\r\n" //行结尾标志

static std::string CodeToInfo(int code){
  std::string Info;
  switch(code){
    case 200:
      Info="OK";
      break;
    case 404:
      Info="NotFound";
      break;
    default:
      break;
  }
  return Info;
}

//HTTP响应报文
class HttpResponse{
  public:
    std::string StatusLine_HTTP;//状态行
    std::vector<std::string>ResponHeads;//首部字段
    std::string ResponBlank=LINE_END;//空行
    std::string ResponBody;//正文

    int status_code=OK;//响应状态码

    int fd=-1;//储存发送网页的文件描述符

    size_t size;//打开网页的大小

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

    std::string Type;//请求文件类型
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
      std::transform(strtmp.begin(),strtmp.end(),strtmp.begin(),::toupper);//写回strtmp首部
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

    int ProceNoCGI(size_t size){
      //构建HTTP响应网页
      //填充状态行
      http_response.fd=open(http_request.Path.c_str(),O_RDONLY);//只读方式打开
      if(http_response.fd>0){
        std::string& Str=http_response.StatusLine_HTTP;
        Str+=VERSON_HTTP;
        Str+=" ";
        Str+=std::to_string(http_response.status_code);
        Str+=" ";
        Str+=CodeToInfo(http_response.status_code);
        Str+=LINE_END;
        http_response.size=size;

        std::string Content_Lenth_str="Content-Lenth: ";
        Content_Lenth_str+=std::to_string(size);
        Content_Lenth_str+=LINE_END;
        http_response.ResponHeads.push_back(Content_Lenth_str);
        std::string Content_Type_str="Content-Type: ";
        Content_Type_str+=Util::SuffixToDesc(http_request.Type);
        Content_Type_str+=LINE_END;
        http_response.ResponHeads.push_back(Content_Type_str);
        return OK;
      }
      return NOTFOUND;
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
      std::string tmpPath;//临时保存请求路径，方便修改
      size_t size =0;//记录文件大小
      struct stat stat_buff;//记录打开文件属性
      size_t suffix_pos=0;//找后缀
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
      if(stat(http_request.Path.c_str(),&stat_buff)==0){
        //资源存在，需要判断这个路径是否访问了路径下的某个资源，如果没有，直接将路径的默认网页响应回去
        if(S_ISDIR(stat_buff.st_mode)){
          //是目录,添加首页信息后还需要重新获取文件stat状态
          http_request.Path+="/";
          http_request.Path+=HOME_PAGE;
          stat(http_request.Path.c_str(),&stat_buff);
        }
        if((stat_buff.st_mode &S_IXUSR)||(stat_buff.st_mode &S_IXGRP)||(stat_buff.st_mode& S_IXOTH)){
          //可执行文件，需要特殊处理
          http_request.CGI=true;
        }
        size=stat_buff.st_size;
      }
      else{
        //资源不存在状态码 404
        ERRORLOG(WARNING,http_request.Path+" Not Found!");
        http_response.status_code=NOTFOUND;
        goto END;
      }

      suffix_pos=http_request.Path.rfind(".");
      if(suffix_pos==std::string::npos){
        http_request.Type=".html";
      }
      else{
        http_request.Type=http_request.Path.substr(suffix_pos);
      }

      if(http_request.CGI==true){
        //ProceCGI();
      }
      else{
        http_response.status_code=ProceNoCGI(size);//一定是GET方法，一定不带参，简单的文本网页返回
      }
END:
      //进行响应
      if(http_response.status_code!=OK){
        //错误响应
        
      }
      return;
    }

    void SendRespon_HTTP(){//发送响应
        send(sock,http_response.StatusLine_HTTP.c_str(),http_response.StatusLine_HTTP.size(),0);
        //std::cout<<"DeBug# "<<http_response.StatusLine_HTTP<<std::endl;
        for(size_t size=0;size<http_response.ResponHeads.size();size++){         
           send(sock,http_response.ResponHeads[size].c_str(),http_response.ResponHeads[size].size(),0);
           //std::cout<<"send succeed"<<std::endl;
        }   //->
        send(sock,http_response.ResponBlank.c_str(),http_response.ResponBlank.size(),0);
        sendfile(sock,http_response.fd,nullptr,http_response.size);
        close(http_response.fd);
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
