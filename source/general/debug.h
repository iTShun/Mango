#ifndef debug_h
#define debug_h

#include "allocator.h"

BEGIN_DECLS

void debug_break();

void debug_output(const char* _out);

void debug_printf_vargs(const char* _format, va_list _argList);

void debug_printf(const char* _format, ...);

void debug_printf_data(const void* _data, uint32_t _size, const char* _format, ...);

END_DECLS

#endif /* debug_h */
