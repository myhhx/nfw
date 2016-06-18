#ifndef nfw_head_table_h
#define nfw_head_table_h

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <boost/thread.hpp>
#include <boost/utility.hpp>
#include <nfw_head.h>

namespace nfw { namespace head {

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
    void listen(
            table_t & table, const std::string & head, handler_t && handler) {
        put(table, head + ":" + typeid(socket_t).name(),
                [handler] (void * socket) {
                    handler((socket_t *) socket);
                });
    }

    template <typename socket_t>
    using adapter_t = std::function<void(socket_t *)>;

    template <typename socket_t, typename table_t>
    adapter_t<socket_t> adapter(table_t & table) {
        return [&table] (socket_t * socket) {
            handler_t handler = get(
                    table, get_head(*socket) + ":" + typeid(socket_t).name());
            if (!handler) {
                delete socket;
            } else {
                handler(socket);
            }
        };
    }

}}

#endif
