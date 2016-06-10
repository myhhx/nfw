#ifndef nfw_link_h
#define nfw_link_h

#include <array>
#include <boost/asio.hpp>
#include <nfw_socket.h>

namespace nfw { namespace link {

    template <typename socket_t>
    void clean(socket_t * socket, bool * flag) {
        if (*flag) {
            *flag = false;
            nfw::socket::close(*socket);
        } else {
            delete flag;
            delete socket;
        }
    }

    template <typename socket_a_t, typename socket_b_t>
    void clean(socket_a_t * a, socket_b_t * b, bool * c, bool * d) {
        nfw::socket::get_io_service(*a).dispatch(
                [b, d] () {
                    clean(b, d);
                });
        clean(a, c);
    }

    typedef std::array<char, 2048> buf_t;

    template <typename socket_a_t, typename socket_b_t>
    void read(socket_a_t * a, socket_b_t * b, bool * c, bool * d, buf_t * buf);

    template <typename socket_a_t, typename socket_b_t>
    void write(socket_a_t * a, socket_b_t * b, bool * c, bool * d, buf_t * buf,
            std::size_t bytes_transferred) {
        boost::asio::async_write(
                *b, boost::asio::buffer(*buf, bytes_transferred),
                [=] (const boost::system::error_code & error, std::size_t) {
                    if (!error) {
                        read(a, b, c, d, buf);
                    } else {
                        delete buf;
                        clean(a, b, c, d);
                    }
                });
    }

    template <typename socket_a_t, typename socket_b_t>
    void read(socket_a_t * a, socket_b_t * b, bool * c, bool * d, buf_t * buf) {
        nfw::socket::async_read_some(*a, boost::asio::buffer(*buf),
                [=] (const boost::system::error_code & error,
                        std::size_t bytes_transferred) {
                    if (!error) {
                        write(a, b, c, d, buf, bytes_transferred);
                    } else {
                        delete buf;
                        clean(b, a, d, c);
                    }
                });
    }

    template <typename socket_a_t, typename socket_b_t>
    void link(socket_a_t * a, socket_b_t * b) {
        bool * c = new bool(true);
        bool * d = new bool(true);
        read(a, b, c, d, new buf_t());
        nfw::socket::get_io_service(*a).dispatch(
                [=] () {
                    read(b, a, d, c, new buf_t());
                });
    }

    template <typename socket_b_t, typename socket_a_t, typename handler_b_t>
    void link(socket_a_t * a, const handler_b_t & b) {
        b([a] (socket_b_t * s) {
            if (s != NULL) {
                link(a, s);
            } else {
                delete a;
            }
        });
    }

    template <typename socket_a_t, typename socket_b_t,
            typename handler_a_t, typename handler_b_t>
    void link(const handler_a_t & a, handler_b_t && b) {
        a([b] (socket_a_t * s) {
            if (s != NULL) {
                link<socket_b_t>(s, b);
            }
        });
    }

}}

#endif
