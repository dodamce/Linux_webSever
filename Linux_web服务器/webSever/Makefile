bin=HttpSever
cc=g++
cgi=test_cgi
LD_FLAGS= -std=c++11 -lpthread
src=main.cpp
curr=$(shell pwd)

BIN:$(bin) $(cgi) 
.PHONY:ALL 

$(bin):$(src)
	$(cc) -o $@ $^ $(LD_FLAGS) 

$(cgi):CGI/testcgi.cpp
	$(cc) -o $@ $^ -std=c++11

.PHONY:clean 
clean:
	rm -f $(bin) $(cgi)
	rm -rf output

.PHONY:output 
output:
	mkdir -p output 
	cp $(bin) output 
	cp -rf wwwroot output 
	cp $(cgi) output/wwwroot
