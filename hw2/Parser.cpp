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

Parser::Parser() {
    addr = "";           
    url = "";            
    http_version = "";
    max_age = "";
    content = "";
    method = "";
    etag = "";
    last_modified = "";
    status_code = "";
    host = "";
    port_number = "";
    no_cache = false; // Request
    no_store = false; // Request
    max_stale = ""; // Request
    min_fresh = ""; // Request
    must_revalidate = false; // Response
    chuncked = false;
    content_length = "";

   
}


void Parser::setArguments(string input, string type) {
    if(type == "Request"){
        // basic paramters
        map<string, string> m;
        string header;
        size_t start = 0;
        size_t end = 0;
        // parse the first line (method, url, http_version)
        end = input.find("\n");
        header = input.substr(start, (end - start));
        vector<string> headerVec = split(header, ' ');
        method = headerVec[0];
        url = headerVec[1];
        http_version = headerVec[2];
        // parse Host
        start = input.find("Host");
        if(start != string::npos) {
            start = start + 6;
            end = input.find("\n", start);
            int col = input.find(":", start);
            if (col != string::npos) {
                host = input.substr(start, (col - start));
                col++;
                port_number = input.substr(col, (end - col));
            }
            else {
                host = input.substr(start, (end - start));
            }
        }
        // parse max-age
        start = input.find("max-age");
        if(start != string::npos){
            start = start + 8;
            end = input.find_first_of(" \n", start);
            max_age = input.substr(start, (end - start));
        }
        //parse max-stale
        start = input.find("max-stale");
        if(start != string::npos){
            start += 10;
            end = input.find_first_of(" \n", start);
            max_stale = input.substr(start, (end - start));
        }       
        //parse min-fresh
        start = input.find("min-fresh");
        if(start != string::npos){
            start += 10;
            end = input.find_first_of(" \n", start);
            min_fresh = input.substr(start, (end - start));
        } 

        // pasrse no-cache
        start = input.find("no-cache");
        if(start != string::npos){
            no_cache = true;
        }
        //parse no-store
        start = input.find("no-store");
        if(start != string::npos){
            no_store = true;
        }        
    }else if(type == "Response"){
        map<string, string> m;
        string header;
        size_t start = 0;
        size_t end = 0;
        // parse the first line(http_version, status_code)
        end = input.find("\n");
        header = input.substr(start, (end - start));
        vector<string> headerVec = split(header, ' ');
        http_version = headerVec[0];
        status_code = headerVec[1];
        // parse Etag
        start = input.find("Etag");
        if(start != string::npos) {
            start = start + 6;
            end = input.find("\n", start);
            etag = input.substr(start, (end - start));
        }
        // parse Last-Modified
        start = input.find("Last-Modified");
        if(start != string::npos) {
            start = start + 15;
            end = input.find("\n", start);
            last_modified = input.substr(start, (end - start));            
        }
        // parse max-age
        start = input.find("max-age");
        if(start != string::npos){
            start = start + 8;
            end = input.find_first_of(" \n", start);
            max_age = input.substr(start, (end - start));
        }
        // parse must_revalidate
        start = input.find("must-revalidate");
        if(start != string::npos){
            must_revalidate = true;
        }   
        // parse content_length
        start = input.find("Content-Length");  
        if(start != string::npos) {
            start += 16;
            end = input.find_first_of("\n", start);
            content_length = input.substr(start, (end - start));
        }      
        // parse transfer encoding -- chuncked
        start = input.find("Transfer-Encoding");
        if(start != string::npos) {
            end = input.find("\n", start);
            string currLine = input.substr(start, (end - start));
            if(currLine.find("chunked") != string::npos) {
                chuncked = true;
            }
        }

    }

}

