#pragma once 

//提供编码过程中所需要的工具函数 

#include<string>
#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<unordered_map>

class Util{
  public:
    static int ReadLine(int sock,std::string&outBuff){
      char ch='D';//ch先随机赋不为'\n'、'\r'的值
      while(ch!='\n'){
        //将三种行结束符统一转化为\n换行
        ssize_t size=recv(sock,&ch,1,0);
        if(size>0){
          if(ch=='\r'){
            //特殊处理 \r->\n   \r\n->\n
            //查看\r后的字符，不取走
            recv(sock,&ch,1,MSG_PEEK);
            if(ch=='\n'){
              //是\r\n换行格式，重复recv即可
              recv(sock,&ch,1,0);//用\n将\r覆盖
            }
            else{
              //是\r的格式，修改ch
              ch='\n';
            }
          }
          outBuff.push_back(ch);//换行格式一定是\n
        }
        else if(size==0){
          return 0;//对端关闭
        }
        else{
          return -1;//读取错误
        }
      }
      return outBuff.size();//返回读取一行字符的个数
    }

    static bool CutString(std::string&src,std::string& key,std::string&value,std::string gist){//根据gist(: )字符串切分HTTP首部字段，将切分的两个字符串放入到map中
      size_t pos=src.find(gist);
      if(pos!=std::string::npos){
        key=src.substr(0,pos);
        value=src.substr(pos+gist.size());//默认截取到字符串尾
        return true;
      }
      return false;
    }

    static std::string SuffixToDesc(const std::string&suffix){
      static std::unordered_map<std::string,std::string>Desc={
        {".html","text/html"},{".css","text/css"},{".js","application/javascript"},
        {".jpg","application/x-jpg"},{".xml","application/xml"},
        {".htm","text/html"},{".ttf","font/ttf"},{".woff","font/woff"},
        {".woff2","font/woff2"}
      };      
      auto pos=Desc.find(suffix);
      if(pos!=Desc.end()){
        return pos->second;
      }
      return "text/html";
    }
};
