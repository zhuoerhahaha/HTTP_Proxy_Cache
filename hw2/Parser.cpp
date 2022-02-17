#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>
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
    port_number = "default";
    no_cache = false; // Request
    no_store = false; // Request
    max_stale = ""; // Request
    min_fresh = ""; // Request
    must_revalidate = false; // Response
    chuncked = false;
    content_length = "";

   
}


void Parser::setArguments(string input, string type) {
    string inputCopy = input;
    transform(inputCopy.begin(), inputCopy.end(),inputCopy.begin(), ::toupper);

    if(type == "Request"){
        // basic paramters
        map<string, string> m;
        string header;
        size_t start = 0;
        size_t end = 0;
        // parse the first line (method, url, http_version)
        end = inputCopy.find("\n");
        header = input.substr(start, (end - start));
        vector<string> headerVec = split(header, ' ');
        method = headerVec[0];
        url = headerVec[1];
        http_version = headerVec[2];
        // parse Host
        start = inputCopy.find("HOST");
        if(start != string::npos) {
            start = start + 6;
            end = inputCopy.find("\n", start);
            string currLine = input.substr(start, (end - start));
            int col = currLine.find(":");
            if (col != string::npos) {
                host = currLine.substr(0, col);
                col++;
                port_number = currLine.substr(col);
            }
            else {
                host = input.substr(start, (end - start) - 1);
            }
        }
        // parse max-age
        start = inputCopy.find("MAX-AGE");
        if(start != string::npos){
            start = start + 8;
            end = inputCopy.find_first_of(" \n", start);
            max_age = input.substr(start, (end - start));
        }
        //parse max-stale
        start = inputCopy.find("MAX-STALE");
        if(start != string::npos){
            start += 10;
            end = inputCopy.find_first_of(" \n", start);
            max_stale = input.substr(start, (end - start));
        }       
        //parse min-fresh
        start = inputCopy.find("MIN-FRESH");
        if(start != string::npos){
            start += 10;
            end = inputCopy.find_first_of(" \n", start);
            min_fresh = input.substr(start, (end - start));
        } 

        // pasrse no-cache
        start = inputCopy.find("NO-CACHE");
        if(start != string::npos){
            no_cache = true;
        }
        //parse no-store
        start = inputCopy.find("NO-STORE");
        if(start != string::npos){
            no_store = true;
        }        
    }else if(type == "Response"){
        map<string, string> m;
        string header;
        size_t start = 0;
        size_t end = 0;
        // parse the first line(http_version, status_code)
        end = inputCopy.find("\n");
        header = input.substr(start, (end - start));
        vector<string> headerVec = split(header, ' ');
        http_version = headerVec[0];
        status_code = headerVec[1];
        // parse Etag
        start = inputCopy.find("ETAG");
        if(start != string::npos) {
            start = start + 6;
            end = inputCopy.find("\n", start);
            etag = input.substr(start, (end - start));
        }
        // parse Last-Modified
        start = inputCopy.find("LAST-MODIFIED");
        if(start != string::npos) {
            start = start + 15;
            end = inputCopy.find("\n", start);
            last_modified = input.substr(start, (end - start));            
        }
        // parse max-age
        start = inputCopy.find("MAX-AGE");
        if(start != string::npos){
            start = start + 8;
            end = inputCopy.find_first_of(" \n", start);
            max_age = input.substr(start, (end - start));
        }
        // parse must_revalidate
        start = inputCopy.find("MUST-REVALIDATE");
        if(start != string::npos){
            must_revalidate = true;
        }   
        // parse content_length
        start = inputCopy.find("CONTENT-LENGTH");  
        if(start != string::npos) {
            start += 16;
            end = inputCopy.find_first_of("\n", start);
            content_length = input.substr(start, (end - start));
        }      
        // parse transfer encoding -- chuncked
        start = inputCopy.find("TRANSFER-ENCODING");
        if(start != string::npos) {
            end = inputCopy.find("\n", start);
            string currLine = input.substr(start, (end - start));
            if(currLine.find("chunked") != string::npos) {
                chuncked = true;
            }
        }
        // parse the date
        start = input.find("DATE");
        if(start != string::npos) {
            start += 6;
            int end = input.find(" GMT", start);
            date = input.substr(start, end);
        }
        

    }

    // cout << "##########\n";
    // cout << host << "3" << endl;
    // cout << "************\n";
}

