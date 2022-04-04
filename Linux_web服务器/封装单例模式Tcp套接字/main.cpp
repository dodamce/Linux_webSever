#include"TcpServer.h"

static void usrHelp(char*name){
  std::cout<<"usrHelp# "<<name<<" +port "<<std::endl;
}

int main(int argc,char*argv[]){
  if(argc!=2){
    usrHelp(argv[0]);
    exit(4);
  }

  int port=atoi(argv[1]);
  TcpSever*inst=TcpSever::GetInstance(port);
  return 0;
}
