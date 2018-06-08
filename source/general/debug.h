#ifndef debug_h
#define debug_h

#include "allocator.h"

BEGIN_DECLS

#define check(_condition, _format, ...)  \
    if (!(_condition))                          \
    {                                           \
        debug_printf("Assert {" __FILE__ "(" STRINGIZE(__LINE__) ")}: " _format "\n", ##__VA_ARGS__); \
        debug_break();                          \
    }

void debug_break();

void debug_output(const char* _out);

void debug_printf_vargs(const char* _format, va_list _argList);

void debug_printf(const char* _format, ...);

void debug_printf_data(const void* _data, uint32_t _size, const char* _format, ...);

END_DECLS

#endif /* debug_h */
