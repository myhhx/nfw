
namespace nfw { namespace _notiman_ {

    template <typename notifier>
    struct soul : std::tuple<

                  std::unordered_map<std::string, notifier>,
                  nfw::shared_mutex> {

        typedef notifier notifier_type;

    };

    template <typename derived, typename base>
    struct body : base {

        function(bool, add_notifier, who, notifier) {
            return get_map().emplace(who, notifier).second;
        }

        function(void, del_notifier, who) {
            get_map().erase(who);
        }

        function(auto, get_notifier, who) {
            notifier_type notifier;

            auto it = get_map().find(who);
            if (it != get_map().end())
                notifier = it->second;

            return notifier;
        }

        function(void, notify, who, what) {

            auto notifier = static_cast<derived *>(this)->get_notifier(who);
            if (!notifier)
                return;

            notifier->notify(what);
        }

    };

    template <typename base>
    struct concurrent : base {
        
        function(bool, add_notifier, who, notifier) {
            return get_mutex().unique_execute([&] () {
                return base::add_notifier(who, notifier);
            });
        }

    };

    template <typename base>
    struct monitor : base {

        function(void, add_notifier, who, notifier) {
            if (base::add_notifier(who, notifier))
                start_monitor(notifier, who);
        }

    };

}}
