#ifndef _DOTA_DEVKIT_HPP_
#define _DOTA_DEVKIT_HPP_

#include <iostream>

#include "http_server.hpp"
#include "devkit_http.hpp"
#include "json.hpp"

using namespace dota;

void testJson() {
    std::string result;
    
    // test string
    std::unordered_map<std::string, json_type> t;
    t["asd2"] = 123123;
    
    std::vector<json_type> t2;
    t2.push_back(std::string("asd1"));
    t2.push_back(std::string("asd2"));
    t2.push_back(std::string("asd3"));
    t2.push_back(std::string("asd4"));
    
    t["t2"] = t2;
    
    json_type obj = t;
    
    jsonToString v(result);
    boost::apply_visitor( v, obj );
    std::cout << result << std::endl;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        testJson();
        std::cout << "Usage: devkit <port> <replay folder>" << std::endl;
        return 1;
    } 
    
    boost::asio::io_service s;
    http_server devkitServer("0.0.0.0", argv[1], new http_request_handler_devkit(argv[2]), s);
    devkitServer.run();

}

#endif //_DOTA_DEVKIT_HPP_