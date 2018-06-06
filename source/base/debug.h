#ifndef debug_h
#define debug_h

#include "platform/platform.h"
#include <stdarg.h>
#include <stdint.h>

void debug_break();

void debug_output(const char* _out);

void debug_printf_vargs(const char* _format, va_list _argList);

void debug_printf(const char* _format, ...);

void debug_printf_data(const void* _data, uint32_t _size, const char* _format, ...);

#endif /* debug_h */
