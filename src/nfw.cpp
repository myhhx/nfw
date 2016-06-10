#include <signal.h>
#include <exception>
#include <functional>
#include <iostream>
#include <boost/asio.hpp>
#include <nfw_arg.h>
#include <nfw_assert.h>
#include <nfw_dg.h>
#include <nfw_head.h>
#include <nfw_link.h>
#include <nfw_sec.h>
#include <nfw_table.h>
#include <nfw_tcp.h>

template <typename socket_t> using handler_t = std::function<void(socket_t *)>;

int main(int argc, char ** argv) {
    using std::placeholders::_1;
    using std::placeholders::_2;
    typedef boost::asio::ip::tcp::socket s0_t;
    typedef nfw::head::socket_t<s0_t> s1_t;
    typedef nfw::dg::socket_t<s1_t> s2_t;
    typedef nfw::sec::socket_t<s2_t> s3_t;
    typedef nfw::head::socket_t<s3_t> s4_t;
    typedef nfw::head::socket_t<s4_t> s5_t;
    try {
        bool client = false;
        int g = nfw::arg::find(argc, argv, "--client");
        if (g >= 0) {
            client = true;
            nfw_assert(g + 4 < argc);
        } else {
            g = nfw::arg::find(argc, argv, "--server");
            nfw_assert(g >= 0 && g + 3 < argc);
        }
        boost::asio::io_service io_service;
        nfw::table::table_t table;
        if (!client) {
            nfw::tcp::accept(io_service, argv[g + 1], atoi(argv[g + 2]),
                    [&] (s0_t * socket) {
                        nfw::head::head_b(
                            socket, nfw::table::adapter<s1_t>(table));
                    });
            nfw::table::listen<s4_t>(table, argv[g + 3],
                    [&] (s4_t * socket) {
                        nfw::head::head_b(
                            socket, nfw::table::adapter<s5_t>(table));
                    });
            nfw::arg::each(argc, argv, "--user",
                    [&] (int i) {
                        nfw_assert(i + 2 < argc);
                        nfw::table::listen<s1_t>(table, argv[i + 1],
                            [&] (s1_t * socket) {
                                nfw::socket::compose<s3_t, s4_t>(
                                        nfw::socket::compose<s2_t, s3_t>(
                                            std::bind(
                                                &nfw::dg::dg<
                                                    s1_t, handler_t<s2_t>>,
                                                socket, _1),
                                            std::bind(
                                                &nfw::sec::handshake_b<
                                                    s2_t, handler_t<s3_t>>,
                                                _1, argv[i + 2], _2)),
                                        std::bind(
                                            &nfw::head::head_b<
                                                s3_t, handler_t<s4_t>>,
                                            _1, _2)
                                    )(nfw::table::adapter<s4_t>(table));
                            });
                    });
        }
        nfw::arg::each(argc, argv, "--link-in",
                [&] (int i) {
                    nfw_assert(i + 3 < argc);
                    nfw::link::link<s5_t, s0_t>(
                        [&] (handler_t<s5_t> && handler) {
                            nfw::table::listen<s5_t>(
                                table, argv[i + 1], handler);
                        },
                        [&] (handler_t<s0_t> && handler) {
                            nfw::tcp::connect(
                                io_service, argv[i + 2], argv[i + 3], handler);
                        });
                });
        nfw::arg::each(argc, argv, "--link-out",
                [&] (int i) {
                    nfw_assert(i + 4 < argc);
                    nfw::link::link<s0_t, s5_t>(
                        std::bind(&nfw::tcp::accept<handler_t<s0_t>>,
                            std::ref(io_service),
                            argv[i + 1], atoi(argv[i + 2]), _1),
                        nfw::socket::compose<s4_t, s5_t>(
                            nfw::socket::compose<s3_t, s4_t>(
                                nfw::socket::compose<s2_t, s3_t>(
                                    nfw::socket::compose<s1_t, s2_t>(
                                        nfw::socket::compose<s0_t, s1_t>(
                                            [&] (handler_t<s0_t> && handler) {
                                                nfw::tcp::connect(io_service,
                                                    argv[g + 1], argv[g + 2],
                                                    handler);
                                            },
                                            std::bind(
                                                &nfw::head::head_a<
                                                    s0_t, handler_t<s1_t>>,
                                                _1, argv[g + 3], _2)),
                                        std::bind(
                                            &nfw::dg::dg<s1_t, handler_t<s2_t>>,
                                            _1, _2)),
                                    std::bind(
                                        &nfw::sec::handshake_a<
                                            s2_t, handler_t<s3_t>>,
                                        _1, argv[g + 4], _2)),
                                std::bind(
                                    &nfw::head::head_a<s3_t, handler_t<s4_t>>,
                                    _1, argv[i + 3], _2)),
                            std::bind(&nfw::head::head_a<s4_t, handler_t<s5_t>>,
                                _1, argv[i + 4], _2)));
                });
        boost::asio::signal_set signals(io_service);
        signals.add(SIGINT);
        signals.add(SIGTERM);
        #ifdef SIGQUIT
        signals.add(SIGQUIT);
        #endif
        signals.async_wait(std::bind(&boost::asio::io_service::stop, &io_service));
        io_service.run();
        return 0;
    } catch (std::exception & e) {
        std::cerr << "exception: " << e.what() << std::endl;
        std::cerr << "usage: nfw "
            << "[--client server_host server_port user_name user_key] "
            << "[--server listen_ip listen_port server_name "
                << "--user user_name user_key --user ...] "
            << "--link-in service_name service_host service_port --link-in ... "
            << "--link-out listen_ip listen_port "
                << "server_name__or__user_name service_name --link-out ..."
            << std::endl;
        return -1;
    }
}
