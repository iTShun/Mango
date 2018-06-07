#include "string.h"

#if !BX_CRT_NONE
#	include <stdio.h> // vsnprintf, vsnwprintf
#	include <wchar.h> // vswprintf
#endif // !BX_CRT_NONE

bool is_space(char _ch)
{
    return ' '  == _ch
    || '\t' == _ch
    || '\n' == _ch
    || '\v' == _ch
    || '\f' == _ch
    || '\r' == _ch
    ;
}

bool is_upper(char _ch)
{
    return _ch >= 'A' && _ch <= 'Z';
}

bool is_lower(char _ch)
{
    return _ch >= 'a' && _ch <= 'z';
}

bool is_alpha(char _ch)
{
    return is_lower(_ch) || is_upper(_ch);
}

bool is_numeric(char _ch)
{
    return _ch >= '0' && _ch <= '9';
}

bool is_alpha_num(char _ch)
{
    return is_alpha(_ch) || is_numeric(_ch);
}

bool is_print(char _ch)
{
    return is_alpha_num(_ch) || is_space(_ch);
}

int32_t vsnprintf_(char* _out, int32_t _max, const char* _format, va_list _argList)
{
	va_list argList;
	va_copy(argList, _argList);
	int32_t total = 0;

#if BX_CRT_MSVC
    int32_t len = -1;
    if (NULL != _out)
    {
        va_list argListCopy;
        va_copy(argListCopy, _argList);
        len = vsnprintf_s(_out, _max, (size_t)(-1), _format, argListCopy);
        va_end(argListCopy);
    }
	total = -1 == len ? _vscprintf(_format, _argList) : len;
#else
	total = vsnprintf(_out, _max, _format, _argList);
#endif // BX_CRT_MSVC

	va_end(argList);
	return total;
}

int32_t vsnwprintf_(wchar_t* _out, int32_t _max, const wchar_t* _format, va_list _argList)
{
	va_list argList;
	va_copy(argList, _argList);
	int32_t total = 0;

#if BX_CRT_MSVC
    int32_t len = -1;
    if (NULL != _out)
    {
        va_list argListCopy;
        va_copy(argListCopy, _argList);
        len = _vsnwprintf_s(_out, _max, (size_t)(-1), _format, argListCopy);
        va_end(argListCopy);
    }
	total = -1 == len ? _vscwprintf(_format, _argList) : len;
#elif BX_CRT_MINGW
	total = vsnwprintf(_out, _max, _format, _argList);
#else
	total = vswprintf(_out, _max, _format, _argList);
#endif // defined(_MSC_VER)

	va_end(argList);
	return total;
}

int32_t snprintf_(char* _out, int32_t _max, const char* _format, ...)
{
    va_list argList;
    va_start(argList, _format);
    int32_t len = vsnprintf_(_out, _max, _format, argList);
    va_end(argList);
    return len;
}

int32_t swnprintf_(wchar_t* _out, int32_t _max, const wchar_t* _format, ...)
{
    va_list argList;
    va_start(argList, _format);
    int32_t len = vsnwprintf_(_out, _max, _format, argList);
    va_end(argList);
    return len;
}
