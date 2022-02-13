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
    string addr = "";           
    string url = "";            
    string http_version = "";
    string max_age = "";
    string content = "";
    string method = "";
    string etag = "";
    string last_modified = "";
    string status_code = "";
    string host = "";
    string port_number = "";
    bool no_cache = false; // Request
    bool no_store = false; // Request
    string max_stale = ""; // Request
    string min_fresh = ""; // Request
    bool must_revalidate = false; // Response

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
            host = input.substr(start, (end - start));
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
    }

}

// Parser::Parser(string input, string type) {
//     string addr = "";           
//     string url = "";            
//     string http_version = "";
//     string content = "";
//     string method = "";
//     string etag = "";
//     string last_modified = "";
//     string status_code = "";
//     string host = "";
//     string port_number = "";
//     // Below are from TA
//     string no_cache = ""; // Request
//     string no_store = ""; // Request
//     string max_stale = ""; // Request
//     string min_fresh = ""; // Request
//     string must_revalidate = ""; // Response
//     string max_age = ""; // Response
    


//     if(type == "Request"){
//         // basic paramters
//         map<string, string> m;
//         string header;
//         string::size_type index;
//         size_t start = 0;
//         size_t end = 0;
//         // parse the first line (method, url, http_version)
//         end = input.find("\n");
//         header = input.substr(start, end);
//         vector<string> headerVec = split(header, ' ');
//         method = headerVec[0];
//         url = headerVec[1];
//         http_version = headerVec[2];
//         // parse Host
//         start = input.find("Host");
//         start = start + 6;
//         end = input.find("\n", start);
//         host = input.substr(start, end);
//         // parse max-age
//         start = input.find("max-age");
//         start = start + 8;
//         if(start != string::npos){
//             end = input.find_first_of(" \n", start);
//             max_age = input.substr(start, end);
//         }
        

//     }else if(type == "Response"){
//         map<string, string> m;
//         string header;
//         string::size_type index;
//         size_t start = 0;
//         size_t end = 0;
//         // parse the first line(http_version, status_code)
//         end = input.find("\n");
//         header = input.substr(start, end);
//         vector<string> headerVec = split(header, ' ');
//         http_version = headerVec[0];
//         status_code = headerVec[1];
//         // parse Etag
//         start = input.find("Etag");
//         start = start + 6;
//         end = input.find("\n", start);
//         etag = input.substr(start, end);
//         // parse Last-Modified
//         start = input.find("Last-Modified");
//         start = start + 15;
//         end = input.find("\n", start);
//         last_modified = input.substr(start, end);
//         // parse max-age
//         start = input.find("max-age");
//         start = start + 8;
//         if(start != string::npos){
//             end = input.find_first_of(" \n", start);
//             max_age = input.substr(start, end);
//         }
//     }
// }