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
    chunked = false;
    content_length = "";
    status_msg = "";
   
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
        end = inputCopy.find("\r\n");
        header = input.substr(start, (end - start));
        vector<string> headerVec = split(header, ' ');
        method = headerVec[0];
        url = headerVec[1];
        http_version = headerVec[2];
        // parse Host
        start = inputCopy.find("HOST");
        if(start != string::npos) {
            start = start + 6;
            end = inputCopy.find("\r\n", start);
            string currLine = input.substr(start, (end - start));
            int col = currLine.find(":");
            if (col != string::npos) {
                host = currLine.substr(0, col);
                col++;
                port_number = currLine.substr(col, (end - start));
            }
            else {
                host = input.substr(start, (end - start));
            }
        }
        //get the cache-control
        start = inputCopy.find("CACHE-CONTROL");
        if(start != string::npos) {
            end = inputCopy.find("\r\n", start);
            string currentLine = input.substr(start, (end - start));
            // parse max-age
            int curr_start = currentLine.find("max-age");
            int curr_end;
            if(curr_start != string::npos){
                curr_start = curr_start + 8;
                curr_end = currentLine.find(",", curr_start);
                if(curr_end != string::npos) {
                    max_age = currentLine.substr(curr_start, (curr_end - curr_start));
                }
                else {
                    max_age = currentLine.substr(curr_start);
                }
            }
            //parse max-stale
            curr_start = currentLine.find("max-stale");
            if(curr_start != string::npos){
                curr_start += 10;
                curr_end = currentLine.find(",", curr_start);
                if(curr_end != string::npos) {
                    max_stale = currentLine.substr(start, (curr_end - curr_start));
                }
                else {
                    max_stale = currentLine.substr(start);
                }
            }       
            //parse min-fresh
            curr_start = currentLine.find("min-fresh");
            if(curr_start != string::npos){
                curr_start += 10;
                curr_end = currentLine.find(",", curr_start);
                if(curr_end != string::npos) {
                    min_fresh = currentLine.substr(curr_start, (curr_end - curr_start));
                }
                else {
                    min_fresh = currentLine.substr(curr_start);
                }
            } 

            // pasrse no-cache
            start = currentLine.find("no-cache");
            if(start != string::npos){
                no_cache = true;
            }
            //parse no-store
            start = currentLine.find("no-store");
            if(start != string::npos){
                no_store = true;
            }  
        }
        
     
    }else if(type == "Response"){
        map<string, string> m;
        string header;
        size_t start = 0;
        size_t end = 0;
        // parse the first line(http_version, status_code)
        end = inputCopy.find("\r\n");
        header = input.substr(start, (end - start));
        vector<string> headerVec = split(header, ' ');
        http_version = headerVec[0];   
        status_code = headerVec[1];
        for(int i = 2; i < headerVec.size(); i++) {
            status_msg.append(headerVec[i] + " ");
        }
        // parse Etag
        start = inputCopy.find("ETAG");
        if(start != string::npos) {
            start = start + 6;
            end = inputCopy.find("\r\n", start);
            etag = input.substr(start, (end - start));
        }
        // parse Last-Modified
        start = inputCopy.find("LAST-MODIFIED");
        if(start != string::npos) {
            start = start + 15;
            end = inputCopy.find("\r\n", start);
            last_modified = input.substr(start, (end - start));            
        }
        
        start = inputCopy.find("CACHE-CONTROL");
        if(start != string::npos) {
            end = inputCopy.find("\r\n", start);
            string currentLine = input.substr(start, (end - start));
            // parse max-age
            int curr_start = currentLine.find("max-age");
            int curr_end;
            if(curr_start != string::npos){
                curr_start += 8;
                curr_end = currentLine.find(",", curr_start);
                if(curr_end != string::npos) {
                    max_age = currentLine.substr(curr_start, (curr_end - curr_start));
                }
                else {
                    max_age = currentLine.substr(curr_start);
                }
                
            }

            // parse must_revalidate
            start = currentLine.find("must-revalidate");
            if(start != string::npos){
                must_revalidate = true;
            }   
        }        
        
        // parse content_length
        start = inputCopy.find("CONTENT-LENGTH");  
        if(start != string::npos) {
            start += 16;
            end = inputCopy.find_first_of("\r\n", start);
            content_length = input.substr(start, (end - start));
        }      
        // parse transfer encoding -- chunked
        start = inputCopy.find("TRANSFER-ENCODING");
        if(start != string::npos) {
            end = inputCopy.find("\r\n", start);
            string currLine = input.substr(start, (end - start));
            if(currLine.find("chunked") != string::npos) {
                chunked = true;
            }
        }
        // parse the date
        start = inputCopy.find("DATE");
        if(start != string::npos) {
            start += 6;
            int end = inputCopy.find(" GMT", start);
            date = input.substr(start, (end - start));
        }
        //parse age
        start = inputCopy.find("AGE: ");
        if(start != string::npos) {
            start += 5;
            end = inputCopy.find("\r\n");
            age = input.substr(start, (end - start));
        }
        //parse expired
        start = inputCopy.find("Expires");
        if(start != string::npos) {
            start += 9;
            end = inputCopy.find("\r\n", start);
            expires = input.find(start, (end - start));
        }
        // cout << input << "\n\n\n" << endl;
    }

    // cout << "##########\n";
    // cout << host << "3" << endl;
    // cout << "************\n";
}

