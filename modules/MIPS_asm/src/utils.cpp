#include "utils.hpp"
#include <cstdarg>
#include <cstdio>

void debugInfo(const char* pattern, ...) {
#ifdef SPLC_DEBUG
    va_list args;
    va_start(args, pattern);
    vprintf(pattern, args);
    va_end(args);
#endif
}

void debugError(const char* pattern, ...) {
#ifdef SPLC_DEBUG
    va_list args;
    va_start(args, pattern);
    vfprintf(stderr, pattern, args);
    va_end(args);
#endif
}
