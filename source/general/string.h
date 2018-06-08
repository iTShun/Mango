#ifndef string_h
#define string_h

#include "allocator.h"
#include <wchar.h>

BEGIN_DECLS

bool is_space(char _ch);

bool is_upper(char _ch);

bool is_lower(char _ch);

bool is_alpha(char _ch);

bool is_numeric(char _ch);

bool is_alpha_num(char _ch);

bool is_print(char _ch);

int32_t strLen(const char* _str, int32_t _max);

int32_t strCopy(char* _dst, int32_t _dstSize, const char* _src, int32_t _num);

int32_t strCat(char* _dst, int32_t _dstSize, const char* _src, int32_t _num);

/// Cross platform implementation of vsnprintf that returns number of
/// characters which would have been written to the final string if
/// enough space had been available.
int32_t vsnprintf_(char* _out, int32_t _max, const char* _format, va_list _argList);

/// Cross platform implementation of vsnwprintf that returns number of
/// characters which would have been written to the final string if
/// enough space had been available.
int32_t vsnwprintf_(wchar_t* _out, int32_t _max, const wchar_t* _format, va_list _argList);

///
int32_t snprintf_(char* _out, int32_t _max, const char* _format, ...);

///
int32_t swnprintf_(wchar_t* _out, int32_t _max, const wchar_t* _format, ...);

END_DECLS

#endif /* string_h */
