CC=g++
CFLAGS=-O3 

all: client server

client: client.cpp
	$(CC) $(CFLAGS) -o $@ client.cpp

server: server.cpp
	$(CC) $(CFLAGS) -o $@ server.cpp

clean:
	rm -f client server *.o 