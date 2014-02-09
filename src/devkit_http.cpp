#include <dirent.h>

#include <boost/lexical_cast.hpp>

#include "json.hpp"
#include "devkit_http.hpp"

namespace dota {
    std::string retOk(json_type t) {
        std::string result;
        jsonToString v(result);
        boost::apply_visitor( v, t );
        
        return std::string("{\"success\":1, \"data\":"+result+"}");
    }
    
    std::string retFail(std::string msg) {
        return std::string("{\"success\":1, \"data\":\""+msg+"\"}");
    }
    
    http_reply http_request_handler_devkit::handle(http_request req) {
        // sanitize our url
        if (req.url.empty() || (req.url[0] != '/') || (req.url.find("..") != std::string::npos) )
            return http_reply::getStock(http_reply::bad_request);

        // reply object for this request
        http_reply r;
        r.status = http_reply::ok;

        // check if this is an api or file request
        if (req.url.substr(1, 3) == "api") {
            // minimum link is /api/00/
            if (req.url.size() < 8)
                return http_reply::getStock(http_reply::bad_request);

            uint32_t method = 0;
            try {
                method = boost::lexical_cast<uint32_t>(req.url.substr(5, 2));
            } catch (std::exception &e) {
                return http_reply::getStock(http_reply::bad_request);
            }

            std::string sessionId = req.getCookie("devkit-session");
            bool gotSession = !sessionId.empty();

            switch (method) {
                case LIST:
                    r.body = methodList();
                    break;
                case OPEN:
                case PARSE:
                case CLOSE:
                case STRINGTABLES:
                case STRINGTABLE:
                case ENTITIES:
                case ENTITY:
                case STATUS:
                case RECV:
                case SEND:
                default:
                    return http_reply::getStock(http_reply::bad_request);
                    break;
            }
            
            r.fields.push_back({"Content-Length", std::to_string(r.body.size())});
            r.fields.push_back({"Connection", "Close"});
        } else {
            // add index.html if path ends in slash
            if (req.url[req.url.size() - 1] == '/')
                req.url += "index.html";

            std::string path = htdocs + req.url;
            std::ifstream is(path.c_str(), std::ios::in | std::ios::binary);

            if (!is)
                return http_reply::getStock(http_reply::not_found);
                
            char buf[4096];

            while (is.read(buf, sizeof(buf)).gcount() > 0)
                r.body.append(buf, is.gcount());

            is.close();

            r.fields.push_back({"Content-Length", std::to_string(r.body.size())});
            r.fields.push_back({"Content-Type", http_mime_type::retrieveFromPath(req.url)});
            r.fields.push_back({"Connection", "Close"});
        }

        return r;
    }
    
    std::string http_request_handler_devkit::methodList() {
        DIR *dir;
        struct dirent *entry;
        std::vector<json_type> entries;
        
        if ((dir = opendir(replaydir.c_str())) == NULL) 
            return retFail("Failed to open directory");
        
        while ((entry = readdir (dir)) != NULL) {
            std::string e(entry->d_name);
            
            if (e.substr(0,1).compare(".") == 0)
                continue;
                
            entries.push_back(std::move(e));
        }
        
        closedir(dir);
        
        return retOk(entries);
    }
}
