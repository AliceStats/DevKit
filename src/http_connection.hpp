#ifndef _HTTP_CONNECTION_HPP_
#define _HTTP_CONNECTION_HPP_

#include <array>
#include <memory>

#include <boost/asio.hpp>

#include "http_request_handler.hpp"
#include "http_request_parser.hpp"
#include "http_reply.hpp"

namespace dota {
    /** Forward declaration for the connection manager */
    class http_connection_manager;
    
    /** A single incoming http connection */
    class http_connection : public std::enable_shared_from_this<http_connection> {
        public:
            /** Construct connection with give socket */
            explicit http_connection(
                boost::asio::ip::tcp::socket &&socket, http_connection_manager &m, http_request_handler *handler
            );
            
            /** Start the first async operation for this connection */
            inline void start() {
                read();
            }
            
            /** Stop all async operations for this conenction */
            inline void stop() {
                socket.close();
            }
        private:
            /** Socket for this connection */
            boost::asio::ip::tcp::socket socket;
            /** This connections manager */
            http_connection_manager &manager;
            /** Handler used to process incoming requests */
            http_request_handler *handler;          
            /** Buffer for incoming data */
            std::array<char, 8192> buffer;
            /** Parser for the incoming request */
            http_parser_interface parser;
            /** Reply to be send back to the client */
            http_reply reply;
            
            /** Perform an async read operation until the data is complete */
            void read();
            /** Perform an async write operation until all data is send */        
            void write();
    };
    
    /** typedef for shared connection ptr */
    typedef std::shared_ptr<http_connection> connectionPtr;
}

#endif