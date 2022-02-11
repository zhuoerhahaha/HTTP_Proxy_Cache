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

Parser::Parser(string input) {
    map<string, string> m;
    string header;
    string::size_type index;
    size_t start = 0;
    size_t end = 0;
    //parse start line
    end = input.find("\n");
    header = input.substr(start, end);
    vector<string> headerVec = split(header, ' ');
    method = headerVec[0];
    url = headerVec[1];
    //parse first line of headers
    start = input.find("Host");
    end = input.find("\n", start);
    vector<string> hostVec = split(input.substr(end, start), ' ');
    end = hostVec[1].find(":") + 1;
    addr = hostVec[1].substr(0, end);
    port = hostVec[1].substr(end);
    //parse 
}