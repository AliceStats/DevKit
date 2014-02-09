#include <string>
#include <vector>
#include <unordered_map>

#include "http_reply.hpp"

namespace dota {
    namespace detail {
        // list of predefined resposes
        std::unordered_map<http_reply::status_type, http_status, std::hash<int> > codes = {
            { http_reply::ok,                    {"HTTP/1.0 200 OK\r\n", "200"} },
            { http_reply::created,               {"HTTP/1.0 201 Created\r\n", "201"} },
            { http_reply::accepted,              {"HTTP/1.0 202 Accepted\r\n", "202"} },
            { http_reply::no_content,            {"HTTP/1.0 204 No Content\r\n", "204"} },
            { http_reply::multiple_choices,      {"HTTP/1.0 300 Multiple Choices\r\n", "300"} },
            { http_reply::moved_permanently,     {"HTTP/1.0 301 Moved Permanently\r\n", "301"} },
            { http_reply::moved_temporarily,     {"HTTP/1.0 302 Moved Temporarily\r\n", "302"} },
            { http_reply::not_modified,          {"HTTP/1.0 304 Not Modified\r\n", "304"} },
            { http_reply::bad_request,           {"HTTP/1.0 400 Bad Request\r\n", "400"} },
            { http_reply::unauthorized,          {"HTTP/1.0 401 Unauthorized\r\n", "401"} },
            { http_reply::forbidden,             {"HTTP/1.0 403 Forbidden\r\n", "403"} },
            { http_reply::not_found,             {"HTTP/1.0 404 Not Found\r\n", "404"} },
            { http_reply::internal_server_error, {"HTTP/1.0 500 Internal Server Error\r\n", "500"} },
            { http_reply::not_implemented,       {"HTTP/1.0 501 Not Implemented\r\n", "501"} },
            { http_reply::bad_gateway,           {"HTTP/1.0 502 Bad Gateway\r\n", "502"} },
            { http_reply::service_unavailable,   {"HTTP/1.0 503 Service Unavailable\r\n", "503"} }
        };
        
        // key - value seperator
        const char separator[] = { ':', ' ' };
        // newline 
        const char crlf[] = { '\r', '\n' };
    }
    
    std::vector<boost::asio::const_buffer> http_reply::asBuffer() {
        // Asio's buffer does not take ownership of the data, hence 
        // the subnamespace containing crlf and seperator.
        
        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::buffer(detail::codes[status].header));
        for (auto &entry : fields) {
            buffers.push_back(boost::asio::buffer(entry.name));
            buffers.push_back(boost::asio::buffer(detail::separator));
            buffers.push_back(boost::asio::buffer(entry.value));
            buffers.push_back(boost::asio::buffer(detail::crlf));
        }
        
        buffers.push_back(boost::asio::buffer(detail::crlf));
        buffers.push_back(boost::asio::buffer(body));
        
        return buffers;
    }
    
    http_reply http_reply::getStock(status_type status) {
        http_reply r;
        
        r.status = status;
        r.body = detail::codes[status].html;
        r.fields.push_back({"Content-Length", std::to_string(r.body.size())});
        r.fields.push_back({"Content-Type", "text/html"});
        r.fields.push_back({"Connection", "Close"});
        
        return r;
    }
} 
