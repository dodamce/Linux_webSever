bin=HttpSever
cc=g++
LD_FLAGS= -std=c++11 -lpthread
src=main.cpp

$(bin):$(src)
	$(cc) -o $@ $^ $(LD_FLAGS)

.PHONY:clean

clean:
	rm -f $(bin)
