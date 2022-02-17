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
#include <thread> 
#include <pthread.h>
#define IN_PORT "12345"  // the port users will be connecting to
#define MAXDATASIZE 700000

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




//erronous!!!
void handle_connect(int server_fd, int client_fd, Parser * input, string content_from_client){
    string str = "HTTP/1.1 200 OK\r\n\r\n";
    send(client_fd, "200 OK", 7, 0);
    fd_set master;    // master file descriptor list
    fd_set temp_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    
    
    //keep track of the biggest file descriptor
    fdmax = server_fd > client_fd ? server_fd + 1 : client_fd + 1;
    int i = 0;
    while(true) {
        FD_ZERO(&master);    // clear the master and temp sets
        FD_SET(client_fd, &master);
        FD_SET(server_fd, &master);
        // add the client socket to the master set
        temp_fds = master;  //copy it
        if(select(fdmax, &temp_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }
        // run through the existing connections looking for data to read
        int len;
        int fd[2] = {server_fd, client_fd};
        for(int i = 0; i < 2; i++) {
            char buff[MAXDATASIZE] = {0};
            if (FD_ISSET(fd[i], &temp_fds)) {        //find the match
                // if(fd[i] == client_fd) {
                //     cout << "FD-------: client_fd";
                // }
                // else {
                //     cout << "FD-------: server_fd";
                // }
                len = recv(fd[i], buff, sizeof(buff) - 1, 0);
                buff[len] = '\0';
                cout << "  Length of string: " << len << endl;
                string str(buff);
                cout << buff << endl;
                if (len <= 0) {
                    return;
                }
                else {
                    if (send(fd[1 - i], buff, len, 0) <= 0) {
                        return;
                    }
                }
            }
            // cout << buff << endl;
        }   
    }

}


// void handle_connect(int server_fd, int client_fd, Parser * input, string content_from_client) {
//   send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
//   fd_set readfds;
//   int nfds = server_fd > client_fd ? server_fd + 1 : client_fd + 1;
//   while (1) {
//     FD_ZERO(&readfds);
//     FD_SET(server_fd, &readfds);
//     FD_SET(client_fd, &readfds);
//     select(nfds, &readfds, NULL, NULL, NULL);
//     int fd[2] = {server_fd, client_fd};
//     int len;
//     for (int i = 0; i < 2; i++) {
//       char message[65536] = {0};
//       if (FD_ISSET(fd[i], &readfds)) {
//         len = recv(fd[i], message, sizeof(message), 0);
//         if (len <= 0) {
//           return;
//         }
//         else {
//           if (send(fd[1 - i], message, len, 0) <= 0) {
//             return;
//           }
//         }
//       }
//     }
//   }
// }






struct tm * getCurrentTime() {
    time_t t; // t passed as argument in function time()
    struct tm * tt; // decalring variable for localtime()
    time (&t); //passing argument to time()
    tt = localtime(&t);
    return tt;
}


pair<string, string> RECEIVE(int server_fd, int client_fd) {
    string header;
    string content;
    Parser *response = new Parser();
    //receive the header from server
    while(true) {
        char currBuff[MAXDATASIZE];
        int numbytes;
        //test
        // if ((numbytes = recv(server_fd, currBuff, MAXDATASIZE-1, 0)) == -1) {
        if ((numbytes = recv(server_fd, currBuff, 1, MSG_WAITALL)) == -1){
            perror("recv");
            exit(1);
        }                                                                                          
        string currStr(currBuff);
        header.append(currStr);
        if(header.find("\r\n\r\n") != string::npos) {
            response->setArguments(header, "Response");
            header = header.substr(0, header.length() - 2);
            content.append("\r\n");
            break;
        }
    }

    cout << header << endl;
    //receive the content from server
    if(response->chuncked) { 
        while(true) {
            char buff[MAXDATASIZE];
            recv(server_fd, buff, sizeof(buff), 0);
            string currentStr = string(buff);
            int end = currentStr.find("\r\n");
            if(end == string::npos) {
                perror("end");
                exit(1);
            }
            int dataLength = stoi(currentStr.substr(0, end), nullptr, 16);
            if(dataLength == 0) {
                break;
            }
            content.append(currentStr);
            // cout << content << endl;
        }


    }
    
    else {
        //deal with content_length-specified data
        // while(true) {
            char currBuff[MAXDATASIZE];
            int numbytes;
            //test
            // if ((numbytes = recv(server_fd, currBuff, MAXDATASIZE-1, 0)) == -1) {
            if ((numbytes = recv(server_fd, currBuff, stoi(response->content_length), MSG_WAITALL)) == -1) {
                perror("recv");
                exit(1);
            }
            // cout << "Number of bytes: " << numbytes << "---------" << endl;
            content.append(currBuff);
            // cout << "String length: " << result.length() << "++++++++++"<< endl;

            // send(client_fd, result.c_str(), result.size(), 0);
            // break;
            // string currStr(currBuff);
            // content.append(currStr);
            // if(content.length() >= stoi(response->content_length)) {
            //     result.append(content);
            //     break;
            // }
        // }

    }
    
    // cout << content.length();
    // cout << content.length() << endl;
    return make_pair(header, content);
}

void SEND(int server_fd, int numBytes_to_send, const char * charToSend) {
    int byteSent = 0;
    do {
        byteSent += send(server_fd, charToSend + byteSent, numBytes_to_send - 1 - byteSent, 0);
    }while(byteSent < numBytes_to_send);
    return;
}


//do not need str_from_client           <url, <<header, content>, <input_time, max_age>>>
void handle_get(int server_fd, int client_fd, Parser * request, string str_from_client, map<string, pair<pair<string, string>, pair<time_t, time_t> > > cache){
    //This is the key of the cache
    string url = request->url;
    if(cache.find(url) == cache.end()) {                    //no such url in map, need to get from server and store the response into cache
        pair<string, string> header_content_pair;           //the header-content string pair of the response from server
        string serverResponse;
        //send the data to server
        if (send(server_fd, str_from_client.c_str(), str_from_client.size(), 0) == -1) {
            perror("send");
        }
        //receive from server, need to use a seperate function to get the response
        header_content_pair = RECEIVE(server_fd, client_fd);

        Parser * response = new Parser();
        //append the header-content string pair to serverString
        serverResponse.append(header_content_pair.first);
        serverResponse.append(header_content_pair.second);
        //send serverString back to the client
        send(client_fd, serverResponse.c_str(), serverResponse.length(), 0);            //need to use a while-loop to send!!!!!
        //parse the response header
        response->setArguments(header_content_pair.first, "Response");
        //get the current time
        //get the current time
        struct tm entry_time; 
        //Mon, 18 Jul 2016 16:06:00 GMT              
        strptime(response->date.c_str(), "%a, %d %b %Y %H:%M:%S", &entry_time);
        
        struct tm entry_age = entry_time;
        //calculate the maximum living time of the cache          
        if(response->max_age != "") {
            entry_age.tm_sec = entry_time.tm_sec + stoi(response->max_age);
        }
        //insert the <url, <<header, content>, <input_time, max_time>>> entry into the cache
        cache.insert({response->url, make_pair(header_content_pair, make_pair(mktime(&entry_time), mktime(&entry_age)))});    //mktime(): struct tm ==>  time_t
    }




    else {                                                  //contains such url, need to check if it expired
        struct tm * current_time = getCurrentTime();
        //get the <<header, content>, <input_time, max_age>> pair
        pair<pair<string, string>, pair<time_t, time_t>> current_responseTime_pair = cache.find(url)->second;
        //max_age - current_time
        if(difftime(current_responseTime_pair.second.second, mktime(current_time)) > 0) {           //fresh data
            string stringToSend;
            //fresh, but need to check if revalidation needed

            stringToSend.append(current_responseTime_pair.first.first);
            //append etag/last_modified to the tail of the header
            if(request->etag != "") {
                stringToSend.append("If-None-Match: " + request->etag + "\r\n");
            }
            if(request->last_modified != "") {
                stringToSend.append("If-Modified-Since: " + request->etag + "\r\n");
            }
            //send the request to the server
            SEND(server_fd, stringToSend.length() + 1, stringToSend.c_str());

            pair<string, string> header_content_pair = RECEIVE(server_fd, client_fd);
            Parser * header = new Parser();
            header->setArguments(header_content_pair.first, "Response");
            if(header->status_code == "200") {                        //the response has been updated
                //get the current time
                struct tm entry_time; 
                //Mon, 18 Jul 2016 16:06:00 GMT              
                strptime(header->date.c_str(), "%a, %d %b %Y %H:%M:%S", &entry_time);
                
                struct tm entry_age = entry_time;
                //calculate the maximum living time of the cache          
                if(header->max_age != "") {
                    entry_age.tm_sec = entry_time.tm_sec + stoi(header->max_age);
                }
                //insert the <url, <<header, content>, <input_time, max_time>>> entry into the cache
                cache[request->url] = make_pair(header_content_pair, make_pair(mktime(&entry_time), mktime(&entry_age)));
                
            }
            else if(header->status_code == "304") {                   //the response has not been updated
                //donothing
            }
        }
        else {
            //expired, need to send the request to server to check if need to update value
            //get the string of response to sent
            string stringToSend = "";
            stringToSend.append(current_responseTime_pair.first.first);
            stringToSend.append(current_responseTime_pair.first.second);
            //send the request to the server
            SEND(server_fd, stringToSend.length() + 1, stringToSend.c_str());
            
            pair<string, string> header_content_pair = RECEIVE(server_fd, client_fd);
            Parser * header = new Parser();
            header->setArguments(header_content_pair.first, "Response");
            //get the current time
            //get the current time
            struct tm entry_time; 
            //Mon, 18 Jul 2016 16:06:00 GMT              
            strptime(header->date.c_str(), "%a, %d %b %Y %H:%M:%S", &entry_time);
                
            struct tm entry_age = entry_time;
            //calculate the maximum living time of the cache          
            if(header->max_age != "") {
                entry_age.tm_sec = entry_time.tm_sec + stoi(header->max_age);
            }
            //insert the <url, <<header, content>, <input_time, max_time>>> entry into the cache
            cache[request->url] = make_pair(header_content_pair, make_pair(mktime(&entry_time), mktime(&entry_age)));
        }
        //send the cached response to client
        string str_toSend_toClient = cache[request->url].first.first + cache[request->url].first.second;
        SEND(client_fd, str_toSend_toClient.length() + 1, str_toSend_toClient.c_str());
    }

}

string handle_post(int server_fd, int client_fd, Parser * input, string str_from_client){
    
    return NULL;

}


bool isNumber(const string& str)
{
    for (char const &c : str) {
        if (std::isdigit(c) == 0) return false;
    }
    return true;
}

//connect to the server
int connectToServer(const char * host, const char * in_port) {
    // const char * port = in_port.c_str();
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;

    string port(in_port);
    if(!isNumber(port)) {
        string port_num = "80";
        // status = getaddrinfo(host, port_num.c_str(), &host_info, &host_info_list);
        status = getaddrinfo(host, "80", &host_info, &host_info_list);     //http
    }
    else {
        status = getaddrinfo(host, in_port, &host_info, &host_info_list);
    }

    if (status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << host << "," << in_port << ")" << endl;
        return -1;
    } //if

    socket_fd = socket(host_info_list->ai_family, 
                host_info_list->ai_socktype, 
                host_info_list->ai_protocol);
    if (socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << host << "," << in_port << ")" << endl;
        return -1;
    } //if
    
    // cout << "Connecting to " << host << " on port " << port_num << "..." << endl;
    
    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        cerr << "Error: cannot connect to socket" << endl;
        cerr << "  (" << host << "," << in_port << ")" << endl;
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

    return sockfd;

}





// void * handle_operation(int new_fd, map<string, pair<string, time_t>> cache) {
//     int send_sockfd;
//     char buf_from_client[MAXDATASIZE];                      //buff to hold the received data from the client
//     char buf_from_server[MAXDATASIZE];                      //buff to hold the received data from the server
//     int numbytes_server;                                    //number of bytes received from the client
//     int numbytes_client;                                    //number of bytes received from the client
    
//     char incomingAddr[INET6_ADDRSTRLEN];

//     //receive from the client
//     if ((numbytes_client = recv(new_fd, buf_from_client, MAXDATASIZE-1, 0)) == -1) {
//         perror("recv");
//         exit(1);
//     }

//     //parse the content from the client
//     string content_from_client = buf_from_client;

//     Parser * input = new Parser();

//     input->setArguments(content_from_client, "Request");       

//     //set up socket and connect to the server
//     send_sockfd = connectToServer(input->host.c_str(), input->port_number.c_str());         

//     string response_content = "";
//     if(input->method == "GET"){
//         handle_get(send_sockfd, new_fd, input, content_from_client, cache);
//     } else if(input->method == "CONNECT"){
//         handle_connect(send_sockfd, new_fd, input, content_from_client);
//     } else if(input->method == "POST"){
//         handle_post(send_sockfd, new_fd, input, content_from_client);
//     } 

//     //close sockfd with server
//     close(send_sockfd);
    
//     //close the current socket for client
//     close(new_fd);
//     exit(0);

// }




int main(void)
{
    int listen_sockfd, new_fd;  // listen to client on listen_sockfd, new connection on new_fd, talk to server on send_sockfd
    int send_sockfd;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    map<string, pair<pair<string, string>, pair<time_t, time_t> > > cache;  //<url, <<header, content>, <input_time, max_time>>>
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

        
        // thread myThread(handle_operation, new_fd, cache);
        if(!fork()) {

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

//need to resolve chunked data reading at 78



//question: CONNECT will return 400
//          connect to server host will break when using http

//when receving chunked data
