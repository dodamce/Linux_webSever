#pragma once 


#include"TcpServer.h"
#include<pthread.h>
#include"Protocol.h"
#include"Log.h"

#define DEF_PORT 8081

class HttpSever{
  private:
    int port;
    bool IsStop;//表明当前服务是否运行
    TcpSever*tcp_inst;
  public:
    HttpSever(int _port=DEF_PORT):port(_port),IsStop(false),tcp_inst(nullptr){}

    void InitHttpSever(){
      tcp_inst=TcpSever::GetInstance(port);
    }

    void Start(){
      ERRORLOG(INFO,"Sever Start!");
      int listen_sock=tcp_inst->GetListenSock();
      while(!IsStop){
        sockaddr_in peer;
        socklen_t len=sizeof(peer);
        int sock=accept(listen_sock,(struct sockaddr*)&peer,&len);
        if(sock<0){
          ERRORLOG(ERROR,"accept error");
          continue;
        }
        ERRORLOG(INFO,"get a new sock");
        int* _sock=new int(sock);//目的让每个线程有自己独立套接字，如果直接给线程sock可能被修改，64位系统下会报警告
        pthread_t tid;
        pthread_create(&tid,nullptr,Entry::SolveQuest,_sock);
        pthread_detach(tid);
      }
    }

    ~HttpSever(){}
};
