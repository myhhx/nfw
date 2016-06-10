#ifndef nfw_table_h
#define nfw_table_h

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <boost/thread.hpp>
#include <boost/utility.hpp>
#include <nfw_head.h>

namespace nfw { namespace table {

    typedef std::function<void(void *)> handler_t;

    struct table_t : boost::noncopyable {
        std::unordered_map<std::string, handler_t> map;
        boost::mutex mutex;
    };

    template <typename handler_t>
    void put(table_t & table, const std::string & head, handler_t && handler) {
        boost::lock_guard<boost::mutex> _(table.mutex);
        table.map[head] = std::move(handler);
    }

    handler_t get(table_t & table, const std::string & head) {
        boost::lock_guard<boost::mutex> _(table.mutex);
        auto it = table.map.find(head);
        if (it != table.map.end()) {
            return it->second;
        } else {
            return handler_t();
        }
    }

    template <typename socket_t, typename table_t, typename handler_t>
    void out(table_t & table, const std::string & head, handler_t && handler) {
        nfw_assert(head.find(':') == std::string::npos);
        put(table, head + ":" + typeid(socket_t).name(),
                [handler] (void * socket) {
                    handler((socket_t *) socket);
                });
    }

    template <typename socket_t, typename table_t, typename handler_t>
    void in(table_t & table, const handler_t & handler) {
        hanlder([&table] (socket_t * socket) {
            if (socket != NULL) {
                const std::string & head = nfw::head::get_head(*socket);
                if (head.find(':') != std::string::npos) {
                    delete socket;
                    return;
                }
                nfw::table::handler_t handler = get(
                        table, head + ":" + typeid(socket_t).name());
                if (handler) {
                    handler(socket);
                } else {
                    delete socket;
                }
            }
        });
    }

    template <typename socket_a_t, typename socket_b_t,
            typename table_t, typename handler_t>
    void outin(
            table_t & table, const std::string & head, handler_t && handler) {
        in<socket_b_t>(table,
                [&] (std::function<void(socket_b_t *)> && h) {
                    out<socket_a_t>(table, head,
                        [h, handler] (socket_a_t * s) {
                            handler(s, std::function<void(socket_b_t *)>(h));
                        });
                });
    }

}}

#endif
