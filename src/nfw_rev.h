#ifndef nfw_rev_h
#define nfw_rev_h

namespace nfw { namespace rev {

    struct table_t : boost::noncopyable {
        std::funtion<boost::asio::io_service &()> get_io_service;
        boost::mutex mutex;
        std::unordered_map<std::string,
            std::tuple<boost::asio::deadline_timer *, bool *,
                nfw::table::handler_t>> map;
    };

    struct write_only_t : boost::noncopyable {
        virtual void write(const char * data, int size) = 0;
        virtual ~write_only_t() {}
    };

    template <typename socket_t>
    struct write_only_imp_t : write_only_t {
        socket_t * socket;
        write_only_imp_t(socket_t * socket);
        void write(const char * data, int size);
        ~write_only_imp_t();
    };

    template <typename socket_t>
    write_only_imp_t<socket_t>::write_only_imp_t(
            socket_t * socket) : socket(socket) {
    }

    template <typename socket_t>
    void write_only_imp_t<socket_t>::write(const char * data, int size) {
    }

    template <typename socket_t>
    write_only_imp_t<socket_t>::~write_only_imp_t() {
    }

    struct notifier_t : boost::noncopyable {
        boost::shared_mutex mutex;
        std::unordered_map<std::string, write_only_t *> map;
    };

    template <typename socket_t>
    void add(notifier_t & notifier, const std::string & user,
            socket_t * socket) {
        auto * write_only = new write_only_imp_t<socket_t>(socket);
        {
            boost::unique_lock<boost::shared_mutex> _(notifier.mutex);
            if (notifier.map.emplace(user, write_only).second) {
                return;
            }
        }
        delete write_only;
    }

    void del(notifier_t & notifier, const std::string & user) {
        write_only_t * write_only;
        {
            boost::unique_lock<boost::shared_mutex> _(notifier.mutex);
            auto it = notifier.map.find(user);
            if (it == notifier.map.end()) {
                return;
            }
            write_only = it->second;
            notifier.map.erase(it);
        }
        delete write_only;
    }

    bool notify(notifier_t & notifier, const std::string & user,
            const char * data, int size) {
        boost::shared_lock<boost::shared_mutex> _(notifier.mutex);
        auto it = notifier.map.find(user);
        if (it == notifier.map.end()) {
            return false;
        }
        it->second->write(data, size);
        return true;
    }

    struct read_only_t : boost::noncopyable {
    };

}}

namespace nfw { namespace table {

    template <typename handler_t>
    void put(nfw::rev::table_t & table, const std::string & head,
            handler_t && handler) {
        boost::lock_guard<boost::mutex> _(table.mutex);
        nfw_assert(table.map.find(head) == table.map.end());
        auto * timer = new boost::asio::deadline_timer(
                table.get_io_service(), boost::posix_time::seconds(3));
        auto * flag = new bool(true);
        timer->async_wait(
                [timer, flag, &table, head] (
                        const boost::system::error_code &) {
                    nfw::table::handler_t handler;
                    {
                        boost::lock_guard<boost::mutex> _(table.mutex);
                        if (*flag) {
                            auto it = table.map.find(head);
                            nfw_assert(it != table.map.end());
                            handler = std::move(std::get<2>(it->second));
                            table.map.erase(it);
                        }
                    }
                    if (handler) {
                        handler(NULL);
                    }
                    delete timer;
                    delete flag;
                });
        auto & tuple = table.map[head];
        std::get<0>(tuple) = timer;
        std::get<1>(tuple) = flag;
        std::get<2>(tuple) = std::move(handler);
    }

    nfw::table::handler_t get(nfw::rev::table_t & table, const std::string & head) {
        nfw::table::handler_t handler;
        boost::lock_guard<boost::mutex> _(table.mutex);
        auto it = table.map.find(head);
        if (it != table.map.end()) {
            handler = std::move(std::get<2>(it->second));
            *(std::get<1>(it->second)) = false;
            std::get<0>(it->second)->cancel();
            table.map.erase(it);
        }
        return handler;
    }

}}

#endif
