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
        return std::string("{\"success\":0, \"data\":\""+msg+"\"}");
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

            // get request method
            uint32_t method = 0;
            try {
                method = boost::lexical_cast<uint32_t>(req.url.substr(5, 2));
            } catch (std::exception &e) {
                return http_reply::getStock(http_reply::bad_request);
            }

            // get request session
            uint32_t session;
            std::string sessionId = req.getCookie("devkit-session");
            if (sessionId.empty()) {
                session = ++count;
            } else {
                try {
                    session = boost::lexical_cast<uint32_t>(sessionId);
                } catch (std::exception &e) {
                    return http_reply::getStock(http_reply::bad_request);
                }
            }
            
            // check if sessions exists
            sessionMutex.lock();
            auto it = sessions.find(session);
            if (it == sessions.end()) {
                sessions[session] = {time(NULL), nullptr};
            } else {
                sessions[session].accessed = time(NULL);
            }
            sessionMutex.unlock();

            switch (method) {
                case LIST:
                    r.body = methodList();
                    break;
                case OPEN:
                    r.body = methodOpen(req.url.substr(8), session);
                    break;
                case PARSE:
                    r.body = methodParse(req.url.substr(8), session);
                    break;
                case CLOSE:
                    r.body = methodClose(session);
                    break;
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
            
            r.fields.push_back({"Set-Cookie", std::string("devkit-session=")+std::to_string(session)});
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
    
    std::string http_request_handler_devkit::methodOpen(std::string arg, uint32_t sId) {
        std::lock_guard<std::mutex> mLock(sessionMutex);
        
        std::string file = replaydir+arg;
        try {
            if (sessions[sId].r != nullptr) {
                auto *mon = sessions[sId].r;
                (*mon)([](reader* r){
                    delete r;
                }).get(); // do this synchronously to keep a valid state
                delete sessions[sId].r;
            }
        
            sessions[sId].r = new monitor<reader*>(new reader(file));
            return retOk(std::string("Replay Opened"));
        } catch (std::exception &e) {
            return retFail(std::string("Failed to open replay with exception: ")+e.what());
        }
    }
    
    std::string http_request_handler_devkit::methodParse(std::string arg, uint32_t sId) {
        sessionMutex.lock();
        auto* mon = sessions[sId].r;
        sessionMutex.unlock();
        
        uint32_t num = 0;
        
        try {
            if (!arg.empty())
                num = boost::lexical_cast<uint32_t>(arg);
        
            if (mon) {
                (*mon)([=](reader* r){
                    for (uint32_t i = 0; i < num; ++i) {
                        r->readMessage();
                    }
                });
                
                return retOk(std::string("Parsing..."));
            } else {
                return retFail("No replay opened");
            }
        } catch (std::exception &e) {
            return retFail(std::string("Failed to parse replay with exception: ")+e.what());
        }
    }
    
    std::string http_request_handler_devkit::methodClose(uint32_t sId) {
        std::lock_guard<std::mutex> mLock(sessionMutex);
        auto* mon = sessions[sId].r;
        
        try {
            if (mon) {
                (*mon)([](reader* r){
                    if (r)
                        delete r;
                }).get(); // do this synchronously to keep a valid state
                delete sessions[sId].r;
                sessions[sId].r = nullptr;
            }
        } catch (std::exception &e) {
            return retFail(std::string("Exception while closing: ")+e.what());
        }
        
        return retOk(std::string("Replay Closed"));
    }
}
