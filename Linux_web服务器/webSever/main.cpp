#include"HttpSever.h"
#include<memory>


static void usrHelp(char*name){
  std::cout<<"usrHelp# "<<name<<" +port "<<std::endl;
}

int main(int argc,char*argv[]){
  if(argc!=2){
    usrHelp(argv[0]);
    exit(4);
  }

  int port=atoi(argv[1]);
  std::shared_ptr<HttpSever>http_sever(new HttpSever(port));
  http_sever->InitHttpSever();
  http_sever->Start();
  return 0;
}
