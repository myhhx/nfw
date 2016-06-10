#ifndef nfw_hpp
#define nfw_hpp

namespace nfw { namespace notification {

    function(void, notify, my, id, msg) {
        shared_lock(my);
        auto it = find(my, id);
        if (it != end(my)) {
            it->second(msg);
        }
    }

    function(void, genrate_notifier, my, id, gen, raw) {
        unique_lock(my);
        emplace(my, id, gen(raw));
    }

    function(void, destory_notifier, my, id) {
        unique_lock(my);
        erase(my, id);
    }

}}

namespace nfw { namespace queue {

    template <typename raw_t>
    struct me {
        raw_t raw;
        std::queue<std::vector<char>> queue;
        bool error;
    };

    function(auto, get_raw, my) {
        return my->raw;
    }

    function(auto, get_queue, my) {
        return my->queue;
    }

    function(void, set_error, my) {
        my->error = true;
    }

    function(bool, is_error, my) {
        return my->error;
    }

    function(void, push, my, buf) {
        push(get_queue(my), buf);
    }

    function(void, pop, my) {
        pop(get_queue(my));
    }

    function(auto, front, my) {
        return front(get_queue(my));
    }

    function(bool, is_empty, my) {
        return is_empty(get_queue(my));
    }

    function(bool, is_writing, my) {
        return is_empty(my);
    }

    function(auto, get_io_service, my) {
        return get_io_service(get_raw(my));
    }

    function(void, dispatch, my, hdl) {
        dispatch(get_io_service(my), hdl);
    }

    function(void, start_write, my) {
        async_write(get_raw(my), front(my), [=] (bool error) {
            if (!error) {
                pop(my);
                if (!is_empty(my)) {
                    start_write(my);
                }
            } else {
                set_error(my);
            }
        });
    }

    function(void, async_write, my, buf, hdl) {
        dispatch(my, [=] () {
            if (!is_error(my)) {
                push(my, buf);
                if (!is_writing(my)) {
                    start_write(my);
                }
                hdl(true);
            } else {
                hdl(false);
            }
        });
    }

}}

#endif
