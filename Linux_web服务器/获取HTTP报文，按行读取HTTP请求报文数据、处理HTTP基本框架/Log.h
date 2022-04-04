#pragma once 

#include<iostream>
#include<string>
#include<time.h>

#define INFO 1
#define WARNING 2
#define ERROE 3
#define FATAL 4

#define ERRORLOG(leve,mesage) ErrorLog(#leve,mesage,__FILE__,__LINE__)  //#作用为，将宏参数转化为字符串

void ErrorLog(std::string Leve,std::string msg,std::string FileName,int LineNum){
  std::cout<<"["<<Leve<<"]"<<"["<<time(NULL)<<"]"<<"["<<msg<<"]"<<"["<<FileName<<"]"<<"["<<LineNum<<"]"<<std::endl;
}
