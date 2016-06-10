#include <exception>
#include <iostream>
#include <boost/asio.hpp>
#include <nfw_arg.h>
#include <nfw_assert.h>
#include <nfw_dg.h>
#include <nfw_head.h>
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
        nfw_assert(argc >= 5);
        boost::asio::io_service io_service;
        nfw::arg::each(argc, argv, "--entry",
                [&io_service, argc, argv] (int i) {
                    nfw_assert(i + 3 < argc);
                    nfw::tcp::accept(io_service, argv[i + 1], atoi(argv[i + 2]),
                        [i, &io_service, argv] (S0 * a) {
                            nfw::tcp::connect(io_service, argv[1], argv[2],
                                [a, i, argv] (S0 * b) {
                                    if (b != NULL) {
                nfw::head::head_a(b, argv[3],
                    [a, i, argv] (S1 * b) {
                        if (b != NULL) {
                            nfw::sec::handshake_a(new S2(b), argv[4],
                                [a, i, argv] (S3 * b) {
                                    if (b != NULL) {
                                        nfw::head::head_a(b, argv[i + 3],
                                            [a] (S4 * b) {
                                                if (b != NULL) {
                                                    nfw::link::link(a, b);
                                                } else {
                                                    delete a;
                                                }
                                            });
                                    } else {
                                        delete a;
                                    }
                                });
                        } else {
                            delete a;
                        }
                    });
                                    } else {
                                        delete a;
                                    }
                                });
                        });
                });
        io_service.run();
        return 0;
    } catch (std::exception & e) {
        std::cerr << "exception: " << e.what() << std::endl;
        std::cerr << "example: nfw_client a.com 12345 user-name user-key"
            << " --entry 0.0.0.0 80 http"
            << " --entry 0.0.0.0 443 https"
            << " --entry 0.0.0.0 22 ssh"
            << std::endl;
        return -1;
    }
}
