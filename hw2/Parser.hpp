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
        std::string no_cache; // Request
        std::string no_store; // Request
        std::string max_stale; // Request
        std::string min_fresh; // Request
        std::string must_revalidate; // Response
        std::string max_age; // Response
        
        Parser(string input, string type);
};