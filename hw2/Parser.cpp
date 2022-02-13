#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
// #include <boost/algorithm/string.hpp>
#include "Parser.hpp"

using namespace std;
vector<string> split(const string &s, char delim) {
    vector<string> elems;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

void request_helper(string input){


}


Parser::Parser(string input, string type) {
    string addr = NULL;           
    string url = NULL;            
    string http_version = NULL;
    string max_age = NULL;
    string content = NULL;
    string method = NULL;
    string etag = NULL;
    string last_modified = NULL;
    string status_code = NULL;
    string host = NULL;
    string port_number = NULL;

    if(type == "Request"){
        // basic paramters
        map<string, string> m;
        string header;
        string::size_type index;
        size_t start = 0;
        size_t end = 0;
        // parse the first line (method, url, http_version)
        end = input.find("\n");
        header = input.substr(start, end);
        vector<string> headerVec = split(header, ' ');
        method = headerVec[0];
        url = headerVec[1];
        http_version = headerVec[2];
        // parse Host
        start = input.find("Host");
        start = start + 6;
        end = input.find("\n", start);
        host = input.substr(start, end);
        // parse max-age
        start = input.find("max-age");
        start = start + 8;
        if(start != string::npos){
            end = input.find_first_of(" \n", start);
            max_age = input.substr(start, end);
        }
        

    }else if(type == "Response"){
        map<string, string> m;
        string header;
        string::size_type index;
        size_t start = 0;
        size_t end = 0;
        // parse the first line(http_version, status_code)
        end = input.find("\n");
        header = input.substr(start, end);
        vector<string> headerVec = split(header, ' ');
        http_version = headerVec[0];
        status_code = headerVec[1];
        // parse Etag
        start = input.find("Etag");
        start = start + 6;
        end = input.find("\n", start);
        etag = input.substr(start, end);
        // parse Last-Modified
        start = input.find("Last-Modified");
        start = start + 15;
        end = input.find("\n", start);
        last_modified = input.substr(start, end);
        // parse max-age
        start = input.find("max-age");
        start = start + 8;
        if(start != string::npos){
            end = input.find_first_of(" \n", start);
            max_age = input.substr(start, end);
        }
    }
}