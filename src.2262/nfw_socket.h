#ifndef nfw_socket_h
#define nfw_socket_h

#include <boost/asio.hpp>

#define NFW_SOCKET(__)                                               \
                                                                     \
namespace nfw { namespace __ {                                       \
                                                                     \
    template <typename raw_t>                                        \
    struct socket_t;                                                 \
                                                                     \
    template <typename raw_t>                                        \
    void cancel(socket_t<raw_t> & socket);                           \
                                                                     \
    template <typename raw_t>                                        \
    void close(socket_t<raw_t> & socket);                            \
                                                                     \
    template <typename raw_t>                                        \
    boost::asio::io_service & get_io_service(                        \
            socket_t<raw_t> & socket);                               \
                                                                     \
    template <typename raw_t, typename buf_t, typename handler_t>    \
    void async_read_some(socket_t<raw_t> & socket,                   \
            const buf_t & buf, handler_t && handler);                \
                                                                     \
    template <typename raw_t, typename buf_t, typename handler_t>    \
    void async_read(socket_t<raw_t> & socket,                        \
            const buf_t & buf, handler_t && handler);                \
                                                                     \
    template <typename raw_t, typename buf_t, typename handler_t>    \
    void async_write(socket_t<raw_t> & socket,                       \
            const buf_t & buf, handler_t && handler);                \
                                                                     \
}}                                                                   \
                                                                     \
namespace nfw { namespace socket {                                   \
                                                                     \
    template <typename raw_t>                                        \
    void cancel(nfw::__::socket_t<raw_t> & socket) {                 \
        nfw::__::cancel(socket);                                     \
    }                                                                \
                                                                     \
    template <typename raw_t>                                        \
    void close(nfw::__::socket_t<raw_t> & socket) {                  \
        nfw::__::close(socket);                                      \
    }                                                                \
                                                                     \
    template <typename raw_t>                                        \
    boost::asio::io_service & get_io_service(                        \
            nfw::__::socket_t<raw_t> & socket) {                     \
        return nfw::__::get_io_service(socket);                      \
    }                                                                \
                                                                     \
    template <typename raw_t, typename buf_t, typename handler_t>    \
    void async_read_some(nfw::__::socket_t<raw_t> & socket,          \
            const buf_t & buf, handler_t && handler) {               \
        nfw::__::async_read_some(socket, buf, handler);              \
    }                                                                \
                                                                     \
}}                                                                   \
                                                                     \
namespace boost { namespace asio {                                   \
                                                                     \
    template <typename raw_t, typename buf_t, typename handler_t>    \
    void async_read(nfw::__::socket_t<raw_t> & socket,               \
            const buf_t & buf, handler_t && handler) {               \
        nfw::__::async_read(socket, buf, handler);                   \
    }                                                                \
                                                                     \
    template <typename raw_t, typename buf_t, typename handler_t>    \
    void async_write(nfw::__::socket_t<raw_t> & socket,              \
            const buf_t & buf, handler_t && handler) {               \
        nfw::__::async_write(socket, buf, handler);                  \
    }                                                                \
                                                                     \
}}                                                                   \

namespace nfw { namespace socket {

    template <typename socket_t>
    void cancel(socket_t & socket) {
        socket.cancel();
    }

    template <typename socket_t>
    void close(socket_t & socket) {
        socket.close();
    }

    template <typename socket_t>
    boost::asio::io_service & get_io_service(
            socket_t & socket) {
        return socket.get_io_service();
    }

    template <typename socket_t, typename buf_t, typename handler_t>
    void async_read_some(socket_t & socket,
            const buf_t & buf, handler_t && handler) {
        socket.async_read_some(buf, handler);
    }

}}
 
NFW_SOCKET(dg)
NFW_SOCKET(head)
NFW_SOCKET(sec)

#endif
