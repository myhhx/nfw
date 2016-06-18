#ifndef nfw_assert_h
#define nfw_assert_h

#include <string.h>
#include <sstream>
#include <stdexcept>

namespace nfw { namespace assert {

    const char * fn(const char * file) {
        #ifdef _WIN32
        const char * p = strrchr(file, '\\');
        #else
        const char * p = strrchr(file, '/');
        #endif
        if (p == NULL) {
            return file;
        } else {
            return p + 1;
        }
    }

    void _(bool result, const char * expr, const char * func,
            const char * file, int line) {
        if (!result) {
            std::ostringstream oss;
            oss << expr
                << " [" << func << ":" << fn(file) << ":" << line << "]";
            throw std::runtime_error(oss.str());
        }
    }

}}

#define nfw_assert(expr) nfw::assert::_(expr, "nfw_assert(" #expr ")", \
        __FUNCTION__, __FILE__, __LINE__)

#endif
