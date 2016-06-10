#ifndef nfw_queue_h
#define nfw_queue_h

namespace nfw { namespace _queue_ {

    template <typename raw>
    struct soul : std::tuple<raw, std::queue<std::vector<char>>, bool> {

        function(auto, get_raw) {
            return get<0>();
        }

        function(auto, get_queue) {
            return get<1>();
        }

        function(bool, is_error) {
            return get<2>();
        }

        function(void, set_error) {
            get<2>() = true;
        }

    };

    template <typename soul>
    struct body : soul {

        function(void, push, buf) {
            get_queue()->push(buf);
        }

        function(void, pop) {
            get_queue()->pop();
        }

        function(auto, front) {
            return get_queue()->front();
        }

        function(bool, is_empty) {
            return get_queue()->is_empty();
        }

        function(void, start_write) {

            get_raw()->async_write(front(), [=] (bool error) {

                if (error) {
                    set_error();
                    return;
                }

                pop();
                if (!is_empty())
                    start_write();
            });
        }

        function(bool, is_writing) {
            return is_empty();
        }

        function(bool, write, buf) {

            if (is_error())
                return false;

            push(buf);
            if (!is_writing())
                start_write();

            return true;
        }

        function(auto, get_io_service) {
            get_raw()->get_io_service();
        }

        function(void, dispatch, hdl) {
            get_io_service()->dispatch(hdl);
        }

        function(void, async_write, buf, hdl) {
            dispatch([=] () {
                hdl(write(buf));
            });
        }

    };

}}

namespace nfw  {
    template <typename raw>
    using queue = _queue_::body<soul<raw>>;
}

#endif
