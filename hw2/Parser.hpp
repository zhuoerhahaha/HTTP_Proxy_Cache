#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <map>

class Parser {
    public:
        std::string addr;           //client address
        std::string port;           //client port number
        std::string method;         //client method
        std::string log_info;       //
        std::string send;
        std::string input;
        std::string line;
        std::string url;            //get url

        Parser(string input);
};