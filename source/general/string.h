#ifndef string_h
#define string_h

#include "allocator.h"
#include <wchar.h>

BEGIN_DECLS

#define STRINGIZE(x) #x

extern bool is_space(char _ch);

extern bool is_upper(char _ch);

extern bool is_lower(char _ch);

extern bool is_alpha(char _ch);

extern bool is_numeric(char _ch);

extern bool is_alpha_num(char _ch);

extern bool is_print(char _ch);

/// Cross platform implementation of vsnprintf that returns number of
/// characters which would have been written to the final string if
/// enough space had been available.
extern int32_t vsnprintf_(char* _out, int32_t _max, const char* _format, va_list _argList);

/// Cross platform implementation of vsnwprintf that returns number of
/// characters which would have been written to the final string if
/// enough space had been available.
extern int32_t vsnwprintf_(wchar_t* _out, int32_t _max, const wchar_t* _format, va_list _argList);

///
extern int32_t snprintf_(char* _out, int32_t _max, const char* _format, ...);

///
extern int32_t swnprintf_(wchar_t* _out, int32_t _max, const wchar_t* _format, ...);

END_DECLS

#endif /* string_h */
