#ifndef _HTTP_SERVER_HPP_
#define _HTTP_SERVER_HPP_

#include <string>
#include <boost/asio.hpp>

#include "http_request_handler.hpp"
#include "http_connection_manager.hpp"

namespace dota {
    /** Very simple embedded HTTP-Server based on Boost.Asio and NodeJS's HttpParser library */
    class http_server {
        public:
            /** Construct a http-server to listen on the specific port and answere requests with the provided handler */
            explicit http_server(
                const std::string& address, const std::string& port, http_request_handler *h, boost::asio::io_service &io
            ) : ioService(io), signals(ioService), acceptor(ioService), manager(), socket(ioService), handler(h)
            {                
                // register to handle termination signals
                signals.add(SIGINT);
                signals.add(SIGTERM);
                #if defined(SIGQUIT)
                    signals.add(SIGQUIT);
                #endif
                
                // handle for termination request
                signals.async_wait([this](boost::system::error_code, int) {        
                    acceptor.close(); 
                    manager.shutdown();
                });
                
                // open new acceptor
                boost::asio::ip::tcp::resolver resolver(ioService);
                boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve({address, port});
                acceptor.open(endpoint.protocol());
                acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
                acceptor.bind(endpoint);
                acceptor.listen();
                
                // list for incoming connections
                accept();
            }
            
            /** Run the http-server */
            inline void run() {
                ioService.run();
            }
        private:
            /** Boost.Asio's io service object */
            boost::asio::io_service &ioService;
            /** Signal set for process termination notifications */
            boost::asio::signal_set signals;
            /** Acceptor listening for incoming connection */
            boost::asio::ip::tcp::acceptor acceptor;
            /** Connection manager, used to terminate outstanding requests */
            http_connection_manager manager;
            /** Next socket to be used for new connection */
            boost::asio::ip::tcp::socket socket;            
            /** Request handler for incoming connections */
            http_request_handler *handler;
            
            /** Accepts a new incoming connection */
            inline void accept() {
                acceptor.async_accept(socket, [this](boost::system::error_code ec) {
                    // check if acceptor is open
                    if (!acceptor.is_open())
                        return;          
                        
                    // An error might occour at this point if the acceptor is closed
                    // between states. This can be ignored because the manager would
                    // not accept the new connection
                        
                    if (!ec)
                        manager.start(std::make_shared<http_connection>(
                            std::move(socket), manager, handler
                        )); 
                        
                    accept();
                });
            }
    };
}
 
#endif /* _BLIB_HTTP_SERVER_HPP_ */
