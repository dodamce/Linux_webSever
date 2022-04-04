#pragma once 

#include<iostream>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include"Log.h"

#define BACKLOG 5

class TcpSever{
  private:
    int port;
    int listen_sock;
    TcpSever(int _port):port(_port),listen_sock(-1){}//单例模式

    TcpSever(const TcpSever&)=delete;

    static TcpSever* inst;
  public:

    int GetListenSock(){return listen_sock;}

    static TcpSever*GetInstance(int _port){
      static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER; //静态锁直接初始化，不需要释放     
      if(inst==nullptr){
        pthread_mutex_lock(&lock);
        if(inst==nullptr){
          inst=new TcpSever(_port);
          inst->InitTcpSever();
        }

        pthread_mutex_unlock(&lock);
      }
      return inst;
    }

    void InitTcpSever(){
      Socket();
      Bind();
      Listen();
      ERRORLOG(INFO,"TcpSever Init Success!!");
    }

    void Socket(){
      listen_sock=socket(AF_INET,SOCK_STREAM,0);
      if(listen_sock<0){
        ERRORLOG(FATA,"sock error");
        exit(1);
      }
      //socket地址复用
      int opt=1;
      setsockopt(listen_sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
      ERRORLOG(INFO,"creat sock...");
    }

    void Bind(){
      struct sockaddr_in local;
      memset(&local,0,sizeof(local));

      local.sin_family=AF_INET;
      local.sin_port=htons(port);
      local.sin_addr.s_addr=INADDR_ANY;
      if(bind(listen_sock,(struct sockaddr*)&local,sizeof(local))<0){
        ERRORLOG(FATA,"bind error");
        exit(2);
      }
      ERRORLOG(INFO,"bind ...");
    }

    void Listen(){
      if(listen(listen_sock,BACKLOG)<0){
        ERRORLOG(FATA,"listen error");
        exit(3);
      }
      ERRORLOG(INFO,"listen ...");
    }

    ~TcpSever(){
      if(listen_sock>=0){
        close(listen_sock);
      }
    }
};

TcpSever*TcpSever::inst=nullptr;
