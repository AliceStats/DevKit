#ifndef _DOTA_DEVKIT_HPP_
#define _DOTA_DEVKIT_HPP_

#include <iostream>

#include "http_server.hpp"
#include "devkit_http.hpp"

using namespace dota;

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cout << "Usage: devkit <port> <replay folder>" << std::endl;
        return 1;
    }

    boost::asio::io_service s;
    http_server devkitServer("0.0.0.0", argv[1], new http_request_handler_devkit(argv[2]), s);
    devkitServer.run();

}

#endif //_DOTA_DEVKIT_HPP_