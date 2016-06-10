#ifndef nfw_arg_h
#define nfw_arg_h

#include <string.h>

namespace nfw { namespace arg {

    int find(int argc, char ** argv, const char * key) {
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], key) == 0) {
                return i;
            }
        }
        return -1;
    }

    template <typename handler_t>
    void each(int argc, char ** argv, const char * key,
            const handler_t & handler) {
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], key) == 0) {
                handler(i);
            }
        }
    }

}}

#endif
