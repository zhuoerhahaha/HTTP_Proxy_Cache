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
#include<string.h>
#define IN_PORT "12345"  // the port users will be connecting to
// #define OUT_PORT "3090"
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


struct tm * getCurrentTime() {
    time_t t; // t passed as argument in function time()
    struct tm * tt; // decalring variable for localtime()
    time (&t); //passing argument to time()
    tt = localtime(&t);
    return tt;
}


//erronous!!!
void handle_connect(int server_fd, int client_fd, Parser * input, string content_from_client){
    // //test
    send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
    char buf[MAXDATASIZE];
    int i = 0;
    while(true) {
        cout << i << endl;
        i++;
        int n_bytes;
        //test--listen to client
        if ((n_bytes = recv(client_fd, buf, MAXDATASIZE-1, 0)) <= 1) {
            perror("recv");
            exit(1);
        }

        cout << "Client says:" << buf[0] << " num bytes " << n_bytes << endl;
        //test--send to server
        // if(send(server_fd, content_from_client.c_str(), content_from_client.size(), 0)== -1){
        //     perror("send");
        //     exit(1);
        // }
        if(send(server_fd, buf, sizeof(buf), 0)== -1){
            perror("send");
            exit(1);
        }
        // vector<unsigned char> buffer(MAXDATASIZE);
        // int n_bytes = recv(sockfd, buffer.data(), buffer.size(), 0);
        //test--listen to server
        if ((n_bytes = recv(server_fd, buf, MAXDATASIZE-1, 0)) <= 1) {
            perror("recv");
            exit(1);
        }
        cout << "Server says:" << buf << " num bytes " << n_bytes << endl;
        //test-send to client
        if(send(client_fd, buf, sizeof(buf), 0)== -1){
            perror("send");
            exit(1);
        }      
        memset(buf, NULL, MAXDATASIZE - 1);  
    }
    
    // if(n_bytes != -1){
    //     // buffer.resize(n_bytes);
    //     buf.resize(n_bytes);
    // } else{
    //     // HANDLE ERROR
    //     perror("receive");
    // }
    // string s(buffer.begin(), buffer.end());
    // string s(buf);
    // cout << endl << n_bytes << s << endl;
    // return s;






    // send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
    // fd_set master;    // master file descriptor list
    // fd_set temp_fds;  // temp file descriptor list for select()
    // int fdmax;        // maximum file descriptor number

    // FD_ZERO(&master);    // clear the master and temp sets
    // FD_SET(client_fd, &master);
    // FD_SET(server_fd, &master);
    
    // //keep track of the biggest file descriptor
    // fdmax = server_fd > client_fd ? server_fd : client_fd;

    // while(true) {
    //     // add the client socket to the master set
    //     temp_fds = master;  //copy it
    //     if(select(fdmax+1, &temp_fds, NULL, NULL, NULL) == -1) {
    //         perror("select");
    //         exit(4);
    //     }
    //     // run through the existing connections looking for data to read
    //     int len;
    //     for(int i = 0; i <= fdmax; i++) {
    //         char buff[MAXDATASIZE];
    //         if (FD_ISSET(i, &temp_fds)) {
    //             len = recv(i, buff, sizeof(buff), 0);
    //             if (len <= 0) {
    //                 return;
    //             }
    //             else {
    //                 if (send(i, buff, len, 0) <= 0) {
    //                     return;
    //                 }
    //             }
    //         }
    //         cout << buff << endl;
    //     }       
    // }



}

string handle_get(int server_fd, int client_fd, Parser * input, string str_from_client, map<string, pair<string, time_t>> cache){
    string url = input->url;
    if(cache.count(url) == 0) {
        //no such url in map, need to get from server and store the response into cache
        char buffFromServer[MAXDATASIZE];
        int numbytes;
        //send the data to server
        if (send(server_fd, str_from_client.c_str(), str_from_client.size(), 0) == -1) {
            perror("send");
        }
        //receive from server
        if ((numbytes = recv(server_fd, buffFromServer, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        Parser * response = new Parser();
        string serverString(buffFromServer);
        response->setArguments(serverString, "Response");
        // time_t current_seconds;
        // current_seconds = time (NULL);
        struct tm * current_time = getCurrentTime();
        if(response->max_age != "") {
            // current_seconds += stoi(response->max_age);
            current_time->tm_sec += stoi(response->max_age);
        }
        cache.insert({response->url, make_pair(serverString, mktime(current_time))});
    }

    else {
        //contains such url, need to check if it expired
        struct tm * current_time = getCurrentTime();
        pair<string, time_t> current_responseTime_pair = cache.find(url)->second;
        
        if(difftime(mktime(current_time), current_responseTime_pair.second) > 0) {
            //non expired, need to check if revalidation needed
            cout << "";
        }
        else {
            //expired, need to send the request to server to check if need to update value

        }

    }
    return NULL;

}

string handle_post(int server_fd, int client_fd, Parser * input, string str_from_client){
    
    return NULL;

}


//connect to the server
int connectToServer(const char * host, const char * port) {
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    //cast port to not null-terminated
    char *port_num = (char *)malloc(strlen(port) - 1);
    memcpy(port_num, port, strlen(port) - 1);


    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    status = getaddrinfo(host, port_num, &host_info, &host_info_list);
    if (status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << host << "," << port_num << ")" << endl;
        return -1;
    } //if

    socket_fd = socket(host_info_list->ai_family, 
                host_info_list->ai_socktype, 
                host_info_list->ai_protocol);
    if (socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << host << "," << port_num << ")" << endl;
        return -1;
    } //if
    
    // cout << "Connecting to " << host << " on port " << port_num << "..." << endl;
    
    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        cerr << "Error: cannot connect to socket" << endl;
        cerr << "  (" << host << "," << port_num << ")" << endl;
        return -1;
    } //if
         

    freeaddrinfo(host_info_list); // all done with this structure

    return socket_fd;
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

    return sockfd;

}


using namespace std::chrono;
int main(void)
{
    int listen_sockfd, new_fd, send_sockfd;  // listen to client on listen_sockfd, new connection on new_fd, talk to server on send_sockfd
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];

    listen_sockfd = setUpServer();

    map<string, pair<string, time_t>> cache;

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
            
            //parse the content from the client
            string content_from_client = buf_from_client;

            Parser * input = new Parser();     

            input->setArguments(content_from_client, "Request");       

            //set up socket and connect to the server
            send_sockfd = connectToServer(input->host.c_str(), input->port_number.c_str());         


            string response_content = "";
            if(input->method == "GET"){
                handle_get(send_sockfd, new_fd, input, content_from_client, cache);
            } else if(input->method == "CONNECT"){
               handle_connect(send_sockfd, new_fd, input, content_from_client);
            } else if(input->method == "POST"){
                handle_post(send_sockfd, new_fd, input, content_from_client);
            } 

            //close sockfd with server
            close(send_sockfd);

            //send back the content to the client
            // if (send(new_fd, response_content.c_str(), response_content.size(), 0) == -1) {
            //     perror("send");
            //     exit(1);
            // }           
            //close the current socket for client
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }

    return 0;
}


//handle_connect: cannot receive anything from server/client, how to implement select

//hendle_get: at 189, implement revalidation for expired/fresh cache data








