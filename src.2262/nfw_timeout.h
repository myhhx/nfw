#ifndef nfw_timeout_h
#define nfw_timeout_h

#include <functional>
#include <boost/asio.hpp>
#include <nfw_socket.h>

namespace nfw { namespace timeout {

    template <typename socket_t>
    boost::asio::deadline_timer * timing(socket_t & socket, int seconds,
            bool * flag) {
        boost::asio::deadline_timer * timer =
                new boost::asio::deadline_timer(
                    nfw::socket::get_io_service(socket),
                    boost::posix_time::seconds(seconds));
        timer->async_wait(
                [timer, &socket, flag] (const boost::system::error_code &) {
                    if (*flag) {
                        *flag = false;
                        nfw::socket::cancel(socket);
                    } else {
                        delete flag;
                    }
                    delete timer;
                });
        return timer;
    }

    typedef std::function<
            void(const boost::system::error_code &, std::size_t)> _;

    template <typename handler_t>
    _ wrap(boost::asio::deadline_timer * timer, handler_t && handler,
            bool * flag) {
        return [timer, handler, flag] (
                const boost::system::error_code & _1, std::size_t _2) {
            if (*flag) {
                *flag = false;
                timer->cancel();
            } else {
                delete flag;
            }
            handler(_1, _2);
        };
    }

    template <typename socket_t, typename handler_t>
    _ wrap(socket_t & socket, int seconds, handler_t && handler) {
        bool * flag = new bool(true);
        boost::asio::deadline_timer * timer = timing(socket, seconds, flag);
        return wrap(timer, handler, flag);
    }

}}

#endif
