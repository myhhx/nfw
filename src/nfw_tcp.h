#ifndef nfw_tcp_h
#define nfw_tcp_h

#include <string>
#include <utility>
#include <boost/asio.hpp>
#include <nfw_assert.h>
#include <nfw_socket.h>

namespace nfw { namespace tcp {

    template <typename handler_t, typename get_io_service_t>
    void accept(boost::asio::ip::tcp::acceptor * acceptor,
            handler_t && handler, get_io_service_t && get_io_service) {
        boost::asio::ip::tcp::socket * socket =
                new boost::asio::ip::tcp::socket(get_io_service());
        acceptor->async_accept(*socket,
                [socket, acceptor, handler, get_io_service] (
                        const boost::system::error_code & error) {
                    nfw_assert(!error);
                    socket->set_option(
                            boost::asio::ip::tcp::no_delay(true));
                    socket->set_option(
                            boost::asio::socket_base::keep_alive(true));
                    handler(socket);
                    accept(acceptor,
                            std::move(handler), std::move(get_io_service));
                });
    }

    template <typename handler_t, typename get_io_service_t>
    void accept(boost::asio::io_service & io_service,
            const std::string & ip, int port,
            handler_t && handler, get_io_service_t && get_io_service) {
        boost::asio::ip::tcp::acceptor * acceptor =
                new boost::asio::ip::tcp::acceptor(io_service,
                    boost::asio::ip::tcp::endpoint(
                        boost::asio::ip::address::from_string(ip), port));
        accept(acceptor, handler, get_io_service);
    }

    template <typename handler_t>
    void accept(boost::asio::io_service & io_service,
            const std::string & ip, int port,
            handler_t && handler) {
        accept(io_service, ip, port, handler,
                [&io_service] () -> boost::asio::io_service & {
                    return io_service;
                });
    }

    template <typename handler_t>
    void connect(boost::asio::io_service & io_service,
            boost::asio::ip::tcp::resolver::iterator it,
            boost::asio::ip::tcp::resolver * resolver,
            handler_t && handler) {
        boost::asio::ip::tcp::socket * socket =
                new boost::asio::ip::tcp::socket(io_service);
        boost::asio::async_connect(*socket, it,
                [socket, resolver, handler] (
                        const boost::system::error_code & error,
                        boost::asio::ip::tcp::resolver::iterator) {
                    if (!error) {
                        socket->set_option(
                                boost::asio::ip::tcp::no_delay(true));
                        socket->set_option(
                                boost::asio::socket_base::keep_alive(true));
                        handler(socket);
                    } else {
                        handler(NULL);
                        delete socket;
                    }
                    delete resolver;
                });
    }

    template <typename handler_t>
    void resolve(boost::asio::io_service & io_service,
            const std::string & host, const std::string & service,
            handler_t && handler) {
        boost::asio::ip::tcp::resolver * resolver =
                new boost::asio::ip::tcp::resolver(io_service);
        resolver->async_resolve(
                boost::asio::ip::tcp::resolver::query(host, service),
                [resolver, &io_service, handler] (
                        const boost::system::error_code & error,
                        boost::asio::ip::tcp::resolver::iterator it) {
                    if (!error) {
                        connect(io_service, it, resolver, std::move(handler));
                    } else {
                        handler(NULL);
                        delete resolver;
                    }
                });
    }

    template <typename handler_t>
    void connect(boost::asio::io_service & io_service,
            const std::string & host, const std::string & service,
            handler_t && handler) {
        resolve(io_service, host, service, handler);
    }

    template <typename handler_t>
    void connect_only_for_bind(boost::asio::io_service & io_service,
            const std::string & host, const std::string & service,
            handler_t && handler) {
        connect(io_service, host, service, handler);
    }

}}

#endif
