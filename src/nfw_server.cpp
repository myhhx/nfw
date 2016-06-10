#include <exception>
#include <iostream>
#include <boost/asio.hpp>
#include <nfw_arg.h>
#include <nfw_assert.h>
#include <nfw_dg.h>
#include <nfw_head.h>
#include <nfw_head_table.h>
#include <nfw_link.h>
#include <nfw_sec.h>
#include <nfw_tcp.h>

int main(int argc, char ** argv) {
    typedef boost::asio::ip::tcp::socket S0;
    typedef nfw::head::socket_t<S0> S1;
    typedef nfw::dg::socket_t<S1> S2;
    typedef nfw::sec::socket_t<S2> S3;
    typedef nfw::head::socket_t<S3> S4;
    try {
        nfw_assert(argc >= 3);
        boost::asio::io_service io_service;
        nfw::head::table_t table;
        nfw::arg::each(argc, argv, "--user",
                [&table, argc, argv] (int i) {
                    nfw_assert(i + 2 < argc);
                    nfw::head::listen<S1>(table, argv[i + 1],
                        [i, &table, argv] (S1 * a) {
                            nfw::sec::handshake_b(new S2(a), argv[i + 2],
                                [&table] (S3 * a) {
                                    if (a != NULL) {
                                        nfw::head::head_b(a,
                                            [&table] (S4 * a) {
                                                if (a != NULL) {
                                            nfw::head::adapter<S4>(table)(a);
                                                }
                                            });
                                    }
                                });
                        });
                });
        nfw::arg::each(argc, argv, "--entry",
                [&table, &io_service, argc, argv] (int i) {
                    nfw_assert(i + 3 < argc);
                    nfw::head::listen<S4>(table, argv[i + 1],
                        [i, &io_service, argv] (S4 * a) {
                            nfw::tcp::connect(
                                io_service, argv[i + 2], argv[i + 3],
                                [a] (S0 * b) {
                                    if (b != NULL) {
                                        nfw::link::link(a, b);
                                    } else {
                                        delete a;
                                    }
                                });
                        });
                });
        nfw::tcp::accept(io_service, argv[1], atoi(argv[2]),
                [&table] (S0 * a) {
                    nfw::head::head_b(a,
                        [&table] (S1 * a) {
                            if (a != NULL) {
                                nfw::head::adapter<S1>(table)(a);
                            }
                        });
                });
        io_service.run();
        return 0;
    } catch (std::exception & e) {
        std::cerr << "exception: " << e.what() << std::endl;
        std::cerr << "example: nfw_server 0.0.0.0 12345 --user name key"
            << " --entry http a.com 80"
            << " --entry https a.com 443"
            << " --entry ssh a.com 22"
            << std::endl;
        return -1;
    }
}
