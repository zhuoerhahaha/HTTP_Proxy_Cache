CC=g++
CFLAGS= -g 

all: proxy

proxy: proxy.cpp 
	$(CC) $(CFLAGS) -o $@ proxy.cpp Parser.cpp  -lpthread

clean:
	rm -f proxy *.o proxy.log