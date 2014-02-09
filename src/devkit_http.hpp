#ifndef _DEVKIT_HTTP_HPP_
#define _DEVKIT_HTTP_HPP_

#include <unordered_map>
#include <atomic>
#include <mutex>

#include <ctime>

#include <boost/lexical_cast.hpp>

#include <alice/reader.hpp>
#include <alice/monitor.hpp>

#include "http_reply.hpp"
#include "http_request.hpp"
#include "http_request_handler.hpp"
#include "http_mime_type.hpp"

#include "config.hpp"

namespace dota {
    /** Devkit http handler */
    class http_request_handler_devkit : public http_request_handler {
        public:
            /** Possible API methods */
            enum methods {
                LIST         =  0, // list all replays available
                OPEN         =  1, // open a specific replay
                PARSE        =  2, // parse X messages
                CLOSE        =  4, // close message
                STRINGTABLES =  5, // get a list of all stringtables
                STRINGTABLE  =  6, // get contents of a single table
                ENTITIES     =  7, // get a list of all entities
                ENTITY       =  8, // get contents of a single entity
                STATUS       =  9, // returns match status
                RECV         = 10, // returns recvprops
                SEND         = 11  // returns sendprops
            };

            /** Struct for a devkit session */
            struct devkit_session {
                /** Last time session was accessed */
                time_t accessed;
                /** Replay reader */
                monitor<reader> r;
            };

            /** Constructor, takes replay directory */
            http_request_handler_devkit(std::string replayDirectory) : htdocs(DEVKIT_HTDOCS), replaydir(replayDirectory) { }

            /** Destructor */
            virtual ~http_request_handler_devkit() { }

            /** handle the http request */
            virtual http_reply handle(http_request req) {
                // sanitize our url
                if (req.url.empty() || (req.url[0] != '/') || (req.url.find("..") != std::string::npos) )
                    return http_reply::getStock(http_reply::bad_request);

                // reply object for this request
                http_reply r;

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

                    r.body = "api";
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

                    r.status = http_reply::ok;
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
        private:
            /** htdocs folder, set via DEVKIT_HTDOCS define */
            std::string htdocs;
            /** replay folder */
            std::string replaydir;
            /** session count */
            std::atomic<uint32_t> count;
            /** session map */
            std::unordered_map<uint32_t, devkit_session> sessions;
            /** mutex for locking / unlocking the session map */
            std::mutex sessionMutex;
    };
}

#endif // _DEVKIT_HTTP_HPP_