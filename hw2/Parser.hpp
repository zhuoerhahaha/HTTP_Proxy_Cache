#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <map>

using namespace std;
class Parser {
    public:
        string addr;           
        string url;            
        string http_version;
        string content;
        string method;
        string etag;
        string last_modified;
        string status_code;
        string status_msg;
        string host;
        string port_number;
        bool no_cache; // Request
        bool no_store; // Request
        string max_stale; // Request
        string min_fresh; // Request
        bool must_revalidate; // Response
        string max_age; // Response
        string content_length;
        bool chunked;
        string date;
        string age;
        string expires;
        
        Parser();
        void setArguments(string input, string type);
};