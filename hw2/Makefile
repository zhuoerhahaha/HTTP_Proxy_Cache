CC=g++
CFLAGS= -Wextra -g 

all: client proxy

client: client.cpp
	$(CC) $(CFLAGS) -o $@ client.cpp

proxy: proxy.cpp 
	$(CC) $(CFLAGS) -o $@ proxy.cpp Parser.cpp  -lpthread

clean:
	rm -f client server proxy *.o proxy.log