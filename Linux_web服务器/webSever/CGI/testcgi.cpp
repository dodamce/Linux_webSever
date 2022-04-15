#include<unistd.h>
#include<stdlib.h>
#include<string>
#include<iostream>
#include"Util.h"

bool GetQuerySrt(std::string& query_str){
  std::string method=getenv("METHOD");
  bool resault=true;
  if(method=="GET"){
    query_str=getenv("QUERY_STRING");
    std::cerr<<"Debug# "<<query_str<<std::endl;
    resault=true;
  }
  else if(method=="POST"){
    int Content_Lenth=atoi(getenv("CONTENT_LENGTH"));
    char ch=0;
    while(Content_Lenth>0){
      read(0,&ch,1);
      query_str.push_back(ch); 
      Content_Lenth--;
    }
    resault=true;
  }
  else{
    resault=false;
  }
  return resault;
}




int main(){
  std::string query_str;
  GetQuerySrt(query_str);

  // a=100&b=200
  std::string str1;std::string str2;
  Util::CutString(query_str,str1,str2,"&");

  std::string key1;std::string value1;
  Util::CutString(str1,key1,value1,"=");

  std::string key2;std::string value2;
  Util::CutString(str2,key2,value2,"=");

  //重定向标准输出，直接向标准输出打印字符，调用方可以通过read读取
  std::cout<<key1<<"->"<<value1<<std::endl;
  std::cout<<key2<<"->"<<value2<<std::endl;
  
  std::cerr<<"Debug:"<<key1<<"->"<<value1<<std::endl;
  std::cerr<<"Debug: "<<key2<<"->"<<value2<<std::endl;
  return 0;
}
