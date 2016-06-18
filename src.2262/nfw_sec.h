#ifndef nfw_sec_h
#define nfw_sec_h

#include <array>
#include <random>
#include <string>
#include <utility>
#include <boost/asio.hpp>
#include <boost/utility.hpp>
#include <boost/uuid/sha1.hpp>
#include <nfw_socket.h>
#include <nfw_timeout.h>

namespace nfw { namespace sec {

    typedef std::mt19937 rand_t;

    template <typename raw_t>
    struct socket_t : boost::noncopyable {
        raw_t * raw;
        socket_t(raw_t * raw) : raw(raw) {
        }
        ~socket_t() {
            delete raw;
        }
        rand_t rand_r;
        rand_t rand_w;
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

    typedef std::array<char, 32> key_t;

    void next_key(rand_t & rand, key_t & key) {
        *((int *) (&key[0]))  = htonl(rand());
        *((int *) (&key[4]))  = htonl(rand());
        *((int *) (&key[8]))  = htonl(rand());
        *((int *) (&key[12])) = htonl(rand());
        *((int *) (&key[16])) = htonl(rand());
        *((int *) (&key[20])) = htonl(rand());
        *((int *) (&key[24])) = htonl(rand());
        *((int *) (&key[28])) = htonl(rand());
    }

    typedef boost::uuids::detail::sha1 md_t;

    void digest(md_t & md, const boost::asio::const_buffer & buffer) {
        md.process_bytes(boost::asio::buffer_cast<const char *>(buffer),
                boost::asio::buffer_size(buffer));
    }

    template <std::size_t n>
    void digest(md_t & md, const std::array<char, n> & array) {
        md.process_bytes(array.data(), array.size());
    }

    typedef std::array<char, 20> digest_t;

    void get_digest(md_t & md, digest_t & digest) {
        unsigned int d[5];
        md.get_digest(d);
        *((int *) (&digest[0]))  = htonl(d[0]);
        *((int *) (&digest[4]))  = htonl(d[1]);
        *((int *) (&digest[8]))  = htonl(d[2]);
        *((int *) (&digest[12])) = htonl(d[3]);
        *((int *) (&digest[16])) = htonl(d[4]);
    }

    template <typename buf_t>
    void digest(rand_t & rand, const buf_t & buf, digest_t & digest) {
        md_t md;
        nfw::sec::digest(md, buf);
        key_t key;
        next_key(rand, key);
        nfw::sec::digest(md, key);
        get_digest(md, digest);
    }

    template <typename raw_t, typename buf_t, typename handler_t>
    void read_again(socket_t<raw_t> & socket,
            const buf_t & buf, handler_t && handler) {
        digest_t * digest = new digest_t();
        boost::asio::async_read(*(socket.raw), boost::asio::buffer(*digest),
                [digest, &socket, buf, handler] (
                        const boost::system::error_code & error,
                        std::size_t bytes_transferred) {
                    if (!error) {
                        digest_t digest_2;
                        nfw::sec::digest(socket.rand_r, buf, digest_2);
                        if (*digest == digest_2) {
                            handler(error, boost::asio::buffer_size(buf));
                        } else {
                            handler(boost::asio::error::no_recovery, -201305);
                        }
                    } else {
                        handler(error, bytes_transferred);
                    }
                    delete digest;
                });
    }

    template <typename raw_t, typename buf_t, typename handler_t>
    void async_read_some(socket_t<raw_t> & socket,
            const buf_t & buf, handler_t && handler) {
        nfw::socket::async_read_some(*(socket.raw), buf,
                [&socket, buf, handler] (
                        const boost::system::error_code & error,
                        std::size_t bytes_transferred) {
                    if (!error) {
                        read_again(socket,
                                boost::asio::buffer(buf, bytes_transferred),
                                std::move(handler));
                    } else {
                        handler(error, bytes_transferred);
                    }
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
    void write_again(socket_t<raw_t> & socket,
            const buf_t & buf, handler_t && handler) {
        digest_t * digest = new digest_t();
        nfw::sec::digest(socket.rand_w, buf, *digest);
        boost::asio::async_write(*(socket.raw), boost::asio::buffer(*digest),
                [digest, buf, handler] (
                        const boost::system::error_code & error,
                        std::size_t bytes_transferred) {
                    if (!error) {
                        handler(error, boost::asio::buffer_size(buf));
                    } else {
                        handler(error, bytes_transferred);
                    }
                    delete digest;
                });
    }

    template <typename raw_t, typename buf_t, typename handler_t>
    void async_write(socket_t<raw_t> & socket,
            const buf_t & buf, handler_t && handler) {
        boost::asio::async_write(*(socket.raw), buf,
                [&socket, buf, handler] (
                        const boost::system::error_code & error,
                        std::size_t bytes_transferred) {
                    if (!error) {
                        write_again(socket, buf, std::move(handler));
                    } else {
                        handler(error, bytes_transferred);
                    }
                });
    }

    typedef std::array<char, 20> uuid_t;

    void uuid(uuid_t & uuid) {
        std::random_device device;
        std::array<char, 32> nums;
        *((int *) (&nums[0]))  = device();
        *((int *) (&nums[4]))  = device();
        *((int *) (&nums[8]))  = device();
        *((int *) (&nums[12])) = device();
        *((int *) (&nums[16])) = device();
        *((int *) (&nums[20])) = device();
        *((int *) (&nums[24])) = device();
        *((int *) (&nums[28])) = device();
        boost::uuids::detail::sha1 sha1;
        sha1.process_bytes(nums.data(), nums.size());
        unsigned int d[5];
        sha1.get_digest(d);
        *((int *) (&uuid[0]))  = d[0];
        *((int *) (&uuid[4]))  = d[1];
        *((int *) (&uuid[8]))  = d[2];
        *((int *) (&uuid[12])) = d[3];
        *((int *) (&uuid[16])) = d[4];
    }

    void seed(rand_t & rand, const uuid_t & uuid, const std::string & key) {
        boost::uuids::detail::sha1 sha1;
        sha1.process_bytes(uuid.data(), uuid.size());
        sha1.process_bytes(key.data(), key.size());
        unsigned int d[5];
        sha1.get_digest(d);
        std::seed_seq seq(d, d + 5);
        rand.seed(seq);
    }

    template <typename raw_t, typename handler_t>
    void a3(socket_t<raw_t> * socket, handler_t && handler) {
        nfw::sec::async_write(*socket, boost::asio::buffer((const void *) 0, 0),
                [socket, handler] (
                        const boost::system::error_code & error, std::size_t) {
                    if (!error) {
                        handler(socket);
                    } else {
                        handler(NULL);
                        delete socket;
                    }
                });
    }

    template <typename raw_t, typename handler_t>
    void a2(socket_t<raw_t> * socket,
            const std::string & key, handler_t && handler) {
        uuid_t * uuid = new uuid_t();
        nfw::sec::async_read(*socket, boost::asio::buffer(*uuid),
                nfw::timeout::wrap(*socket, 2,
                [uuid, socket, key, handler] (
                        const boost::system::error_code & error, std::size_t) {
                    if (!error) {
                        seed(socket->rand_w, *uuid, key);
                        a3(socket, std::move(handler));
                    } else {
                        handler(NULL);
                        delete socket;
                    }
                    delete uuid;
                }));
    }

    template <typename raw_t, typename handler_t>
    void a1(socket_t<raw_t> * socket,
            const std::string & key, handler_t && handler) {
        uuid_t * uuid = new uuid_t();
        nfw::sec::uuid(*uuid);
        seed(socket->rand_r, *uuid, key);
        boost::asio::async_write(*(socket->raw), boost::asio::buffer(*uuid),
                [uuid, socket, key, handler] (
                        const boost::system::error_code & error, std::size_t) {
                    if (!error) {
                        a2(socket, key, std::move(handler));
                    } else {
                        handler(NULL);
                        delete socket;
                    }
                    delete uuid;
                });
    }

    template <typename raw_t, typename handler_t>
    void handshake_a(raw_t * raw,
            const std::string & key, handler_t && handler) {
        a1(new socket_t<raw_t>(raw), key, handler);
    }

    template <typename raw_t, typename handler_t>
    void b3(socket_t<raw_t> * socket, handler_t && handler) {
        nfw::sec::async_read(*socket, boost::asio::buffer((void *) 0, 0),
                nfw::timeout::wrap(*socket, 2,
                [socket, handler] (
                        const boost::system::error_code & error, std::size_t) {
                    if (!error) {
                        handler(socket);
                    } else {
                        handler(NULL);
                        delete socket;
                    }
                }));
    }

    template <typename raw_t, typename handler_t>
    void b2(socket_t<raw_t> * socket,
            const std::string & key, handler_t && handler) {
        uuid_t * uuid = new uuid_t();
        nfw::sec::uuid(*uuid);
        seed(socket->rand_r, *uuid, key);
        nfw::sec::async_write(*socket, boost::asio::buffer(*uuid),
                [uuid, socket, handler] (
                        const boost::system::error_code & error, std::size_t) {
                    if (!error) {
                        b3(socket, std::move(handler));
                    } else {
                        handler(NULL);
                        delete socket;
                    }
                    delete uuid;
                });
    }

    template <typename raw_t, typename handler_t>
    void b1(socket_t<raw_t> * socket,
            const std::string & key, handler_t && handler) {
        uuid_t * uuid = new uuid_t();
        boost::asio::async_read(*(socket->raw), boost::asio::buffer(*uuid),
                nfw::timeout::wrap(*(socket->raw), 2,
                [uuid, socket, key, handler] (
                        const boost::system::error_code & error, std::size_t) {
                    if (!error) {
                        seed(socket->rand_w, *uuid, key);
                        b2(socket, key, std::move(handler));
                    } else {
                        handler(NULL);
                        delete socket;
                    }
                    delete uuid;
                }));
    }

    template <typename raw_t, typename handler_t>
    void handshake_b(raw_t * raw,
            const std::string & key, handler_t && handler) {
        b1(new socket_t<raw_t>(raw), key, handler);
    }

}}

#endif
