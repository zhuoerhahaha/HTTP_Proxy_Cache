#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <map>

class Parser {
    public:
        std::string addr;           
        std::string url;            
        std::string http_version;
        std::string max_age;
        std::string content;
        std::string method;
        std::string etag;
        std::string last_modified;
        std::string status_code;
        std::string host;
        std::string port_number;
        Parser(string input, string type);
};