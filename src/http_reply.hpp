#ifndef _HTTP_REPLY_HPP_
#define _HTTP_REPLY_HPP_

#include <map>
#include <string>
#include <vector>

#include <boost/asio.hpp>

namespace dota {
    /** Reply header field. */
    struct http_field {
        /** Name of the field, e.g. Connection */
        std::string name;
        /** Value of the field, e.g. Closed */
        std::string value;
    };
    
    /** Reply struct */
    struct http_status {
        /** Header e.g. 200 ok */
        std::string header;
        /** Request body */
        std::string html;
    };
    
    /** Response to a http request */
    class http_reply {
        public:        
            /** Status tupe */
            enum status_type {
                ok = 200,
                created = 201,
                accepted = 202,
                no_content = 204,
                multiple_choices = 300,
                moved_permanently = 301,
                moved_temporarily = 302,
                not_modified = 304,
                bad_request = 400,
                unauthorized = 401,
                forbidden = 403,
                not_found = 404,
                internal_server_error = 500,
                not_implemented = 501,
                bad_gateway = 502,
                service_unavailable = 503
            } status = ok;
            
            /** Headers to be included in reply */
            std::vector<http_field> fields;
            /** Body */
            std::string body = "";
            
            /** Returns buffer of content */
            std::vector<boost::asio::const_buffer> asBuffer();
            /** Returns stock reply of status */
            static http_reply getStock(status_type);
    };
}

#endif /* _HTTP_REPLY_HPP_ */