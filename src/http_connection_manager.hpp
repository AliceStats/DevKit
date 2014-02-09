#ifndef _HTTP_CONNECTION_MANAGER_HPP_
#define _HTTP_CONNECTION_MANAGER_HPP_

#include <set>
#include "http_connection.hpp"

namespace dota {
    /** Keeps track of all connections */
    class http_connection_manager {
        public:
            /** Add specified connection to instance and start it */
            inline void start(connectionPtr p) {
                connections.insert(p);
                p->start();
            }
            
            /** Stop the specified connection */
            inline void stop(connectionPtr p) {
                connections.erase(p);
                p->stop();
            }
            
            /** Stop all connections */
            inline void shutdown() {
                for (auto c : connections)
                    c->stop();
                    
                connections.clear();
            }
        private:
            /** List of all managed connections */
            std::set<connectionPtr> connections;
    };
}

#endif // _HTTP_CONNECTION_MANAGER_HPP_