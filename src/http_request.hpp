#ifndef _HTTP_REQUEST_HPP_
#define _HTTP_REQUEST_HPP_

#include <string>
#include <unordered_map>
#include <memory>

namespace dota {    
    /** Http request object returned by the http parser */
    class http_request {        
        public:
            /** The url request */
            std::string url;
            /** Data send (e.g. post) */
            std::string body;
            /** All fields and their values */
            std::unordered_map<std::string, std::string> fields;
            
            /** Constructor */
            http_request() : url(""), body(""), fields() { }
            
            /** Move constructor */
            inline http_request(http_request&& r) {
                url = std::move(r.url);
                body = std::move(r.body);
                fields = std::move(r.fields);
            }
            
            /** Returns cookie if available */
            inline std::string getCookie(const std::string& name) {
                if (fields.find("Cookie") == fields.end()) 
                    return "";
                    
                std::string cookie = fields["Cookie"];                    
                std::size_t cStart = cookie.find(name);
                if (cStart == std::string::npos)
                    return "";
                    
                cStart = cStart+1+name.size();
                if (cStart >= cookie.size())
                    return "";
                
                std::size_t cEnd = cookie.substr(cStart).find(";");
                if (cEnd == std::string::npos) 
                    return cookie.substr(cStart);
                else                
                    return cookie.substr(cStart, cEnd);
            }
            
            /** 
             * Helper function to decode a url.
             * 
             * @todo: implement
             */
            static std::string urlDecode(std::string&& url) {
                return url;
            }
    };
}

#endif /* _HTTP_REQUEST_HPP_ */