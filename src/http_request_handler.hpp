#ifndef _HTTP_REQUEST_HANDLER_HPP_
#define _HTTP_REQUEST_HANDLER_HPP_

#include <string>

#include "http_request.hpp"
#include "http_reply.hpp"

namespace dota {
    /** Parent class for our request handler */
    class http_request_handler {
        public:
            /** Empty constructor */
            http_request_handler() {}      
            
            /** Empty virtual destructor */
            virtual ~http_request_handler() { }   
            
            /** Handle method to be overloaded by definite handler */
            virtual http_reply handle(http_request req) = 0;            
    };
}

#endif // _HTTP_REQUEST_HANDLER_HPP_