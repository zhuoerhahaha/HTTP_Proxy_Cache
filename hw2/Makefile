CC=g++
CFLAGS=-O3 

all: client proxy

client: client.cpp
	$(CC) $(CFLAGS) -o $@ client.cpp

proxy: proxy.cpp Parser.hpp
	$(CC) $(CFLAGS) -o $@ proxy.cpp

clean:
	rm -f client server proxy *.o 