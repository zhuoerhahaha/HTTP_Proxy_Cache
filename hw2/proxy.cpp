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
#include <mutex> 
#include <fstream>
#include <sys/stat.h>
#include <syslog.h>

#define IN_PORT "12345"  // the port users will be connecting to
#define MAXDATASIZE 700000


#define BACKLOG 10   // how many pending connections queue will hold
ofstream logFile("./proxy.log");
using namespace std;

mutex mtx;

static void skeleton_daemon()
{
    // pre-fork
    pid_t pid = fork();
    
    /* An error occurred */
    if (pid < 0){
        perror("pid < 0");
        exit(EXIT_FAILURE);
    }    
    else if(pid == 0){
        /* On success: The child process becomes session leader */
        if (setsid() < 0)
        exit(EXIT_FAILURE);
        /* Catch, ignore and handle signals */
        /*TODO: Implement a working signal handler */
        signal(SIGHUP, SIG_IGN);
        /* Fork off for the second time*/
        pid = fork();
        if(pid < 0){
            perror("pid < 0");
            exit(EXIT_FAILURE);
        }
          /* Set new file permissions */
        umask(0);
        /* Change the working directory to the root directory */
        /* or another appropriated directory */
        chdir("/");

        /* Close all open file descriptors */
        int x;
        for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
        {
            close (x);
        }

        /* Open the log file */
        openlog ("firstdaemon", LOG_PID, LOG_DAEMON);
    }     
    
    
}


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

string getThreadID() {
    auto myid = this_thread::get_id();
    stringstream ss;
    ss << myid;
    return ss.str();
}

void LOG(string msg) {
    mtx.lock();
    cout << msg;
    logFile << msg << endl;
    mtx.unlock();
}

void handle_connect(int server_fd, int client_fd, Parser * input, string content_from_client){
    string str = "HTTP/1.1 200 OK\r\n\r\n";
    send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 21, 0);
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
                if(fd[i] == client_fd) {
                    // cout << "FD-------: client_fd";
                }
                else {
                    // cout << "FD-------: server_fd";
                }
                len = recv(fd[i], buff, sizeof(buff) - 1, 0);
                // cout << "  Length of string: " << len << endl;
                string str(buff);
                // cout << "I received: " << buff << "\n" << endl;
                if (len <= 0) {
                    LOG(getThreadID() + ": Tunnel closed\n");
                    return;
                }
                else {
                    int byteSent = 0;
                    do {
                        int num = send(fd[1 - i], buff + byteSent, len - byteSent, 0);
                        byteSent += num;
                        if(num <= 0) {
                            LOG(getThreadID() + ": Tunnel closed");
                            return;
                        }
                    }while(byteSent < len);
                }
            }
            // cout << buff << endl;
        }   
    }

}






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

    // cout << "RECEIVING..." << endl;

    char currBuff[MAXDATASIZE];
    if (recv(server_fd, currBuff, sizeof(currBuff), 0) == -1){
        perror("recv");
        exit(1);
    } 
    string str_from_server;
    str_from_server.append(currBuff);
    int start = str_from_server.find("\r\n\r\n");
    // cout << currBuff + start << endl;
    if(start == string::npos) {
        "------------------WIERD+++++++++++++++";
    }
    start += 2;
    header = str_from_server.substr(0, start);
    // cout << header << endl;
    response->setArguments(header, "Response");
    memset(currBuff, 0, sizeof(currBuff));
    if(response->chunked) {
        content.append(str_from_server.substr(start));
        while(true) {
            memset(currBuff, 0, sizeof(currBuff));
            if(recv(server_fd, currBuff, sizeof(currBuff), 0) == 0) {
                break;
            }
            string currentStr = string(currBuff);
            content.append(currentStr);

            if(currentStr.find("0\r\n\r\n") != string::npos) {
                break;
            }
        }
        cout << content.length() << endl;
    } 
    else if(response->content_length != "") {
        content = str_from_server.substr(start);
        int already_received = content.length() - 2;
        int byteLeft = stoi(response->content_length) - already_received;
        if(byteLeft > 0) {
            recv(server_fd, currBuff, byteLeft, MSG_WAITALL);
        }
        content.append(currBuff);
    }


    // cout << content << endl;
    // cout << content << endl;

    return make_pair(header, content);
}

void SEND(int server_fd, int numBytes_to_send, const char * charToSend) {
    int byteSent = 0;
    do {
        int num = send(server_fd, charToSend + byteSent, numBytes_to_send - 1 - byteSent, 0);
        byteSent += num;
        if(num <= 0) {
            break;
        }
    }while(byteSent < numBytes_to_send);
    return;
}


//do not need str_from_client           <url, <<header, content>, <input_time, max_age>>>
void handle_get(int server_fd, int client_fd, Parser * request, string str_from_client, map<string, pair<pair<string, string>, pair<time_t, time_t> > > &cache){
    // remove_from_cache(cache);
    //This is the key of the cache
    string url = request->url;
    //update cache length

    mtx.lock();
    if(cache.find(url) == cache.end()) {                    //no such url in map, need to get from server and store the response into cache
        mtx.unlock();
        LOG(getThreadID() + ": not in cache\n");
        pair<string, string> header_content_pair;           //the header-content string pair of the response from server
        string serverResponse;
        //send the data to server
        if (send(server_fd, str_from_client.c_str(), str_from_client.size(), 0) == -1) {
            perror("send");
        }
        LOG(getThreadID() + ": Requesting \"" + request->method + " " + request->url + " " + request->http_version + "\" from " + request->host + "\n");
        //receive from server, need to use a seperate function to get the response
        header_content_pair = RECEIVE(server_fd, client_fd);
        
        Parser * response = new Parser();
        //append the header-content string pair to serverString
        serverResponse.append(header_content_pair.first);
        serverResponse.append(header_content_pair.second);
        //parse the response header
        response->setArguments(header_content_pair.first, "Response");
        LOG(getThreadID() + ": Received \"" + response->http_version + " " + response->status_code + " " + response->status_msg + "\" from " + request->url + "\n");
        if(response->status_code != "200" || response->no_store) {
            LOG(getThreadID() + ": not cacheable because " + response->status_code + "\n");
            SEND(client_fd, serverResponse.length() + 1, serverResponse.c_str());
            LOG(getThreadID() + ": Responding \"" + response->http_version + " " + response->status_code + " " + response->status_msg + "\n");
            return;
        }
        struct tm entry_time; 
        //Mon, 18 Jul 2016 16:06:00 GMT              
        strptime(response->date.c_str(), "%a, %d %b %Y %H:%M:%S", &entry_time);
        entry_time.tm_hour -= 5;
        char buf[50];
        time_t tm = mktime(&entry_time);
        // cout << "entry time: " << ctime_r(&tm, buf);
        struct tm entry_age = entry_time;
        //calculate the maximum living time of the cache          
        if(response->max_age != "") {
            entry_age.tm_sec = entry_time.tm_sec + stoi(response->max_age);
            if(response->age != "") {
                entry_age.tm_sec -= stoi(response->age);
            }
        }
        //insert the <url, <<header, content>, <input_time, max_time>>> entry into the cache
        mtx.lock();
        cache.insert(make_pair(url, make_pair(header_content_pair, make_pair(mktime(&entry_time), mktime(&entry_age)))));    //mktime(): struct tm ==>  time_t
        mtx.unlock();
            // cout << "Return from handle" << endl;
    }




    else {                                                  //contains such url, need to check if it expired
        // cout << "Already in cache" << endl;
        mtx.unlock();
        struct tm * current_time = getCurrentTime();
        struct tm expired_time; 
        //get the <<header, content>, <input_time, max_age>> pair
        pair<pair<string, string>, pair<time_t, time_t>> current_responseTime_pair = cache[url];
        //max_age - current_time
        Parser * cached_header = new Parser();
        //Mon, 18 Jul 2016 16:06:00 GMT 
        if(cached_header->expires == "") {
            expired_time = *current_time;
            expired_time.tm_year += 2000;
        }      
        else {
            strptime(cached_header->expires.c_str(), "%a, %d %b %Y %H:%M:%S", &expired_time);
            expired_time.tm_hour -= 5;
            cached_header->setArguments(current_responseTime_pair.first.first, "Response"); 
        }       
        bool must_revalidate = cached_header->no_cache;
        cout <<"EXPIRES AT: " << asctime(&expired_time);




        if(must_revalidate || (difftime(current_responseTime_pair.second.second, mktime(current_time)) <= 0) || (difftime(mktime(&expired_time), mktime(current_time)) <= 0) ) {           //for revalidation
            struct tm* expiredAt;
            if(difftime(mktime(&expired_time), current_responseTime_pair.second.second) < 0 ) {
                expiredAt = &expired_time;
            }
            else {
                expiredAt = gmtime(&current_responseTime_pair.second.second);
            }
            LOG(getThreadID() + ": in cache, but expired at " + asctime(expiredAt));
            // cout << "Need validation!!" << endl;
            string stringToSend;
            //append the header
            str_from_client = str_from_client.substr(0, (str_from_client.length() - 2));
            stringToSend.append(str_from_client);
            // cout << cached_header->last_modified << endl;

            //append etag/last_modified to the tail of the header
            if(cached_header->etag != "") {
                // cout << "append etags" << endl;
                stringToSend.append("If-None-Match: " + cached_header->etag + "\r\n");
            }
            if(cached_header->last_modified != "") {
                // cout << "append date" << endl;
                stringToSend.append("If-Modified-Since: " + cached_header->last_modified + "\r\n");
            }
            LOG(getThreadID() + ": in cache, requires validation\n");
            
            stringToSend.append("\r\n"); 
            // cout << stringToSend << endl;
            send(server_fd, stringToSend.c_str(), stringToSend.length(), 0);
            LOG(getThreadID() + ": Requesting \"" + request->method + " " + request->url + " " + request->http_version + "\" from " + request->host + "\n");
            pair<string, string> header_content_pair = RECEIVE(server_fd, client_fd);
            Parser * header = new Parser();
            header->setArguments(header_content_pair.first, "Response");
            LOG(getThreadID() + ": Received \"" + header->http_version + " " + header->status_code + " " + header->status_msg + "\" from " +request->host + "\n");
            // cout << "Status code: " << header->status_code << endl;
            if(header->status_code == "200") {                        //the response has been updated
                //get the current time
                struct tm entry_time; 
                //Mon, 18 Jul 2016 16:06:00 GMT              
                strptime(header->date.c_str(), "%a, %d %b %Y %H:%M:%S", &entry_time);
                entry_time.tm_hour -= 5;
                struct tm entry_age = entry_time;
                //calculate the maximum living time of the cache          
                if(header->max_age != "") {
                    entry_age.tm_sec = entry_time.tm_sec + stoi(header->max_age);
                    if(header->age != "") {
                        entry_age.tm_sec -= stoi(header->age);
                    }
                }
                //insert the <url, <<header, content>, <input_time, max_time>>> entry into the cache
                mtx.lock();
                cache[request->url] = make_pair(header_content_pair, make_pair(mktime(&entry_time), mktime(&entry_age)));
                mtx.unlock();

            }
            else if(header->status_code == "304") {                   //the response has not been updated
                LOG(getThreadID() + ": cached no need to update" + "\n");
                //donothing
            }
            else {
                LOG(getThreadID() + ":  not cacheable because " + header->status_code + "\n");
                string str_to_send = header_content_pair.first + header_content_pair.second;
                SEND(client_fd, str_to_send.length() + 1, str_to_send.c_str());
                LOG(getThreadID() + ": Responding \"" + header->http_version + " " + header->status_code + " " + header->status_msg + "\n");
                return;
            }
            LOG(getThreadID() + ": Responding \"" + header->http_version + " " + header->status_code + " " + header->status_msg + "\n");
        }
        else {
            LOG(getThreadID() + ": in cache, valid\n");
            
            //non-stale, directly send to the client
            //get the string of response to sent
            //do nothing but the log
        }
    }

    mtx.lock();
    string str_toSend_toClient = cache[url].first.first + cache[url].first.second;
    // cout << "sending......" << endl;
    mtx.unlock();
    SEND(client_fd, str_toSend_toClient.length() + 1, str_toSend_toClient.c_str());

}

void handle_post(int server_fd, int client_fd, Parser * input, string str_from_client){
    if(send(server_fd, str_from_client.c_str(), str_from_client.size(), 0) == -1){
        perror("send");
    }
    pair<string, string> header_content_pair;
    string serverResponse;
    //receive from server, need to use a seperate function to get the response
    header_content_pair = RECEIVE(server_fd, client_fd);
    serverResponse.append(header_content_pair.first);
    serverResponse.append(header_content_pair.second);
    send(client_fd, serverResponse.c_str(), serverResponse.length(), 0);
    cout << "POST finished!" << endl;

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





void handleThread(int client_fd, int server_fd, map<string, pair<pair<string, string>, pair<time_t, time_t> > > &cache, Parser * input, string content_from_client, const char * ip_addr) {
    string response_content = "";
    struct tm * nowTime = getCurrentTime();

    string log = getThreadID() + ": \"" + input->method + " " + input->url + " " + input->http_version + "\" from " + ip_addr + " @ " + asctime(nowTime);
    LOG(log);
    if(input->method == "GET"){
        handle_get(server_fd, client_fd, input, content_from_client, cache);
    } else if(input->method == "CONNECT"){
        handle_connect(server_fd, client_fd, input, content_from_client);
    } else if(input->method == "POST"){
        handle_post(server_fd, client_fd, input, content_from_client);
    } 
    close(client_fd);
    close(server_fd);
    return;
}




int main(void)
{
    skeleton_daemon();
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
        // printf("server: got connection from %s\n", s);

        


        char buf_from_client[MAXDATASIZE];                      //buff to hold the received data from the client
        char buf_from_server[MAXDATASIZE];                      //buff to hold the received data from the server
        int numbytes_server;                                    //number of bytes received from the client
        int numbytes_client;                                    //number of bytes received from the client
        char incomingAddr[INET6_ADDRSTRLEN];

        //receive from the client
        if ((numbytes_client = recv(new_fd, buf_from_client, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }

        //parse the content from the client
        string content_from_client = buf_from_client;
        Parser * header = new Parser();

        header->setArguments(content_from_client, "Request");       

        //set up socket and connect to the server
        send_sockfd = connectToServer(header->host.c_str(), header->port_number.c_str());         

        thread(handleThread, new_fd, send_sockfd, ref(cache), header, content_from_client, s).detach();
        // myThread.detach();
    }
    close(listen_sockfd);
    syslog (LOG_NOTICE, "First daemon terminated.");
    closelog();
    return 0;
}


//handle_connect: cannot receive anything from server/client, how to implement select

//hendle_get: at 189, implement revalidation for expired/fresh cache data

//need to resolve chunked data reading at 78



//question: CONNECT will return 400
//          connect to server host will break when using http

//when receving chunked data
