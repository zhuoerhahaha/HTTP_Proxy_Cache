#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <chrono>
#include <iostream>
#include "Parser.hpp"
#include <vector>
#define IN_PORT "12345"  // the port users will be connecting to
#define OUT_PORT "5490"
#define MAXDATASIZE 6000

#define BACKLOG 10   // how many pending connections queue will hold
using namespace std;
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

string handle_connect(int sockfd, string ip_address_server){
    if(send(sockfd, ip_address_server.c_str(), sizeof(ip_address_server), 0)== -1){
        perror("send");
        exit(1);
    }
    vector<unsigned char> buffer(MAXDATASIZE);
    int n_bytes = recv(sockfd, buffer.data(), buffer.size(), 0);
    if(n_bytes != -1){
        buffer.resize(n_bytes);
    } else{
        // HANDLE ERROR
        perror("receive");
    }
    string s(buffer.begin(), buffer.end());
    return s;
}

string handle_get(int sockfd, string ip_address_server){
    
    return NULL;

}

string handle_post(int sockfd, string ip_address_server){
    
    return NULL;

}


//connect to the server
int connectToServer(const char * address, char * incomingAddr) {
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(address, OUT_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    // incomingAddr = s;                                                                       //point incomingAddr to s, how??????
    // printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    return sockfd;
}

//set up as a server
int setUpServer() {
    int sockfd;  // listen on sock_fd
    struct addrinfo hints, *servinfo, *p;
    struct sigaction sa;
    int yes=1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, IN_PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");


}


using namespace std::chrono;
int main(void)
{
    int listen_sockfd, new_fd, send_sockfd;  // listen to client on listen_sockfd, new connection on new_fd, talk to server on send_sockfd
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    

    listen_sockfd = setUpServer();

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(listen_sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            char buf_from_client[MAXDATASIZE];                      //buff to hold the received data from the client
            char buf_from_server[MAXDATASIZE];                      //buff to hold the received data from the server
            int numbytes_server;                                    //number of bytes received from the client
            int numbytes_client;                                    //number of bytes received from the client
            close(listen_sockfd);                                   // child doesn't need the listener
            char incomingAddr[INET6_ADDRSTRLEN];

            //receive from the client
            if ((numbytes_client = recv(new_fd, buf_from_client, MAXDATASIZE-1, 0)) == -1) {
                perror("recv");
                exit(1);
            }
            
            Parser * input = new Parser(buf_from_client);                    //parse the content from the client
        
            //print the get message from client
            std::cout << buf_from_client << " from " << s << " @ ";        //need to support time
            
            //set up socket and connect to the server
            const char * str = NULL;
            send_sockfd = connectToServer(str, incomingAddr);           //question: how to pass in the string as parameter and let the string point to another string

            //send request to the server
            // if (send(send_sockfd, "127.0.0.1", 9, 0) == -1) {
            //     perror("send");
            //     exit(1);
            // }

            string response_content = "";
            if(input->method == "GET"){
                response_content = handle_get(send_sockfd, input->url);
            } else if(input->method == "CONNECT"){
               response_content =  handle_connect(send_sockfd, input->url);
            } else if(input->method == "POST"){
                response_content = handle_post(send_sockfd, input->url);
            } 

            //get the request from the server
            if ((numbytes_server = recv(send_sockfd, buf_from_server, MAXDATASIZE-1, 0)) == -1) {
                perror("recv");
                exit(1);
            }
            close(send_sockfd);

            //send back the content to the client
            if (send(new_fd, response_content.c_str(), response_content.size(), 0) == -1) {
                perror("send");
                exit(1);
            }           
            //close the current socket for client
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}