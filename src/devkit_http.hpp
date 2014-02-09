#ifndef _DEVKIT_HTTP_HPP_
#define _DEVKIT_HTTP_HPP_

#include <unordered_map>
#include <atomic>
#include <mutex>
#include <ctime>

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
                CLOSE        =  3, // close message
                STRINGTABLES =  4, // get a list of all stringtables
                STRINGTABLE  =  5, // get contents of a single table
                ENTITIES     =  6, // get a list of all entities
                ENTITY       =  7, // get contents of a single entity
                STATUS       =  8, // returns match status
                RECV         =  9, // returns recvprops
                SEND         = 10  // returns sendprops
            };

            /** Struct for a devkit session */
            struct devkit_session {
                /** Last time session was accessed */
                time_t accessed;
                /** Replay reader */
                monitor<reader*>* r;
            };

            /** Constructor, takes replay directory */
            http_request_handler_devkit(std::string replayDirectory) 
                : htdocs(DEVKIT_HTDOCS), replaydir(replayDirectory), count(0) { }

            /** Destructor */
            virtual ~http_request_handler_devkit() { }

            /** handle the http request */
            virtual http_reply handle(http_request req);
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
            
            /** Returns result of LIST API call */
            std::string methodList();
            /** Returns result of OPEN API call */
            std::string methodOpen(std::string arg, uint32_t sId);
            /** Returns result of PARSE API call */
            std::string methodParse(std::string arg, uint32_t sId);
            /** Returns result of CLOSE API call */
            std::string methodClose(uint32_t sId);
    };
}

#endif // _DEVKIT_HTTP_HPP_