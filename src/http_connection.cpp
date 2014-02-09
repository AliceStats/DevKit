#include <utility>

#include "http_connection_manager.hpp"
#include "http_connection.hpp"

namespace dota {
    http_connection::http_connection(
        boost::asio::ip::tcp::socket &&socket, http_connection_manager &m, http_request_handler *handler
    ) : socket(std::move(socket)), manager(m), handler(handler), buffer(), parser(), reply() { }
            
    void http_connection::read() {
        auto self = shared_from_this(); // keeps connection open as long as the read takes
        socket.async_read_some(boost::asio::buffer(buffer), [this, self](boost::system::error_code e, std::size_t transferred) {
            if (!e) {
                switch (parser.parse(buffer.data(), transferred)) {
                    case http_parser_interface::good:
                        reply = handler->handle(parser.getRequest());
                        write();
                        break;
                    case http_parser_interface::bad:
                        reply = http_reply::getStock(http_reply::bad_request);
                        write();
                        break;
                    default:
                        read();
                }
            } else if (e != boost::asio::error::operation_aborted) {
                manager.stop(shared_from_this());
            }
        });
    }
            
    void http_connection::write() {
        auto self = shared_from_this(); // keeps connection open as long as the write rakes
        boost::asio::async_write(socket, reply.asBuffer(), [this, self](boost::system::error_code e, std::size_t) {
            if (!e) {
                boost::system::error_code ignored;
                socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored);
            }
            
            if (e != boost::asio::error::operation_aborted) {
                manager.stop(shared_from_this());
            }
        });
    }
}