#ifndef nfw_dg_h
#define nfw_dg_h

#include <array>
#include <utility>
#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <nfw_socket.h>

namespace nfw { namespace dg {

    template <typename raw_t>
    struct socket_t : boost::noncopyable {
        raw_t * raw;
        socket_t(raw_t * raw) : raw(raw) {
        }
        ~socket_t() {
            delete raw;
        }
    };

    template <typename raw_t>
    void cancel(socket_t<raw_t> & socket) {
        nfw::socket::cancel(*(socket.raw));
    }

    template <typename raw_t>
    void close(socket_t<raw_t> & socket) {
        nfw::socket::close(*(socket.raw));
    }

    template <typename raw_t>
    boost::asio::io_service & get_io_service(
            socket_t<raw_t> & socket) {
        return nfw::socket::get_io_service(*(socket.raw));
    }

    typedef std::array<char, 4> head_t;

    template <typename raw_t, typename buf_t, typename handler_t>
    void async_read_some(socket_t<raw_t> & socket,
            const buf_t & buf, handler_t && handler) {
        head_t * head = new head_t();
        boost::asio::async_read(*(socket.raw), boost::asio::buffer(*head),
                [head, &socket, buf, handler] (
                        const boost::system::error_code & error,
                        std::size_t bytes_transferred) {
                    if (!error) {
                        int size = ntohl(*((int *) head->data()));
                        if (size <= boost::asio::buffer_size(buf)) {
                            boost::asio::async_read(*(socket.raw),
                                    boost::asio::buffer(buf, size),
                                    std::move(handler));
                        } else {
                            handler(boost::asio::error::message_size, -201305);
                        }
                    } else {
                        handler(error, bytes_transferred);
                    }
                    delete head;
                });
    }

    template <typename raw_t, typename buf_t, typename handler_t>
    void async_read(socket_t<raw_t> & socket,
            const buf_t & buf, handler_t && handler) {
        async_read_some(socket, buf,
                [buf, handler] (
                        const boost::system::error_code & error,
                        std::size_t bytes_transferred) {
                    if (!error) {
                        if (bytes_transferred == boost::asio::buffer_size(buf)) {
                            handler(error, bytes_transferred);
                        } else {
                            handler(boost::asio::error::message_size, -201305);
                        }
                    } else {
                        handler(error, bytes_transferred);
                    }
                });
    }

    template <typename raw_t, typename buf_t, typename handler_t>
    void async_write(socket_t<raw_t> & socket,
            const buf_t & buf, handler_t && handler) {
        head_t * head = new head_t();
        *((int *) head->data()) = htonl(boost::asio::buffer_size(buf));
        boost::asio::async_write(*(socket.raw), boost::asio::buffer(*head),
                [head, &socket, buf, handler] (
                        const boost::system::error_code & error,
                        std::size_t bytes_transferred) {
                    if (!error) {
                        boost::asio::async_write(*(socket.raw), buf,
                                std::move(handler));
                    } else {
                        handler(error, bytes_transferred);
                    }
                    delete head;
                });
    }

}}

#endif
