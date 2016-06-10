#ifndef nfw_head_h
#define nfw_head_h

#include <string.h>
#include <array>
#include <string>
#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <nfw_assert.h>
#include <nfw_socket.h>
#include <nfw_timeout.h>

namespace nfw { namespace head {

    template <typename raw_t>
    struct socket_t : boost::noncopyable {
        raw_t * raw;
        std::string head;
        socket_t(raw_t * raw, const std::string & head) : raw(raw), head(head) {
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

    template <typename raw_t, typename buf_t, typename handler_t>
    void async_read_some(socket_t<raw_t> & socket,
            const buf_t & buf, handler_t && handler) {
        nfw::socket::async_read_some(*(socket.raw), buf, handler);
    }

    template <typename raw_t, typename buf_t, typename handler_t>
    void async_read(socket_t<raw_t> & socket,
            const buf_t & buf, handler_t && handler) {
        boost::asio::async_read(*(socket.raw), buf, handler);
    }

    template <typename raw_t, typename buf_t, typename handler_t>
    void async_write(socket_t<raw_t> & socket,
            const buf_t & buf, handler_t && handler) {
        boost::asio::async_write(*(socket.raw), buf, handler);
    }
 
    template <typename raw_t>
    raw_t & get_raw(socket_t<raw_t> & socket) {
        return *(socket.raw);
    }

    template <typename raw_t>
    const std::string & get_head(socket_t<raw_t> & socket) {
        return socket.head;
    }

    typedef std::array<char, 64> head_t;

    template <typename raw_t, typename handler_t>
    void head_a(raw_t * raw, const std::string & head, handler_t && handler) {
        head_t * h = new head_t();
        nfw_assert(strlen(head.c_str()) < h->size());
        strcpy(h->data(), head.c_str());
        boost::asio::async_write(*raw, boost::asio::buffer(*h),
                [h, raw, handler] (
                        const boost::system::error_code & error, std::size_t) {
                    if (!error) {
                        handler(new socket_t<raw_t>(raw, h->data()));
                    } else {
                        handler(NULL);
                        delete raw;
                    }
                    delete h;
                });
    }

    template <typename raw_t, typename handler_t>
    void head_b(raw_t * raw, handler_t && handler) {
        head_t * h = new head_t();
        boost::asio::async_read(*raw, boost::asio::buffer(*h),
                nfw::timeout::wrap(*raw, 2,
                [h, raw, handler] (
                        const boost::system::error_code & error, std::size_t) {
                    if (!error && memchr(h->data(), 0, h->size())) {
                        handler(new socket_t<raw_t>(raw, h->data()));
                    } else {
                        handler(NULL);
                        delete raw;
                    }
                    delete h;
                }));
    }

    template <typename raw_t, typename handler_t>
    void head(
            raw_t * raw, const std::string & head, const handler_t & handler) {
        handler(new socket_t<raw_t>(raw, head));
    }
 
}}

#endif
