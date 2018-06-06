#include "debug.h"
#include "string.h"
#include <inttypes.h>  // PRIx*

#if PLATFORM_ANDROID
#   include <malloc.h>
#    include <android/log.h>
#elif PLATFORM_WINDOWS || PLATFORM_WINRT || PLATFORM_XBOX360 || PLATFORM_XBOXONE
#   include <malloc.h>
extern __declspec(dllimport) void __stdcall OutputDebugStringA(const char* _str);
#elif PLATFORM_IOS || PLATFORM_OSX
#   include <malloc/malloc.h>
#   if PLATFORM_OSX
#       include <alloca.h>
#   endif
#    if defined(__OBJC__)
#        import <Foundation/NSObjCRuntime.h>
#    else
#        include <CoreFoundation/CFString.h>
extern void NSLog(CFStringRef _format, ...);
#    endif // defined(__OBJC__)
#elif PLATFORM_EMSCRIPTEN
#   include <malloc.h>
#    include <emscripten.h>
#else
#    include <stdio.h> // fputs, fflush
#endif // PLATFORM_WINDOWS

void debug_break()
{
#if defined(_MSC_VER)
    __debugbreak();
#elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM)
    __builtin_trap();
    //        asm("bkpt 0");
#elif !PLATFORM_NACL && (defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__x86_64__)) && (defined(__GNUC__) || defined(__clang__))
    // NaCl doesn't like int 3:
    // NativeClient: NaCl module load failed: Validation failure. File violates Native Client safety rules.
    __asm__ ("int $3");
#else // cross platform implementation
    int* int3 = (int*)3L;
    *int3 = 3;
#endif //
}

void debug_output(const char* _out)
{
#if PLATFORM_ANDROID
#    ifndef ANDROID_LOG_TAG
#        define ANDROID_LOG_TAG ""
#    endif // ANDROID_LOG_TAG
    __android_log_write(ANDROID_LOG_DEBUG, ANDROID_LOG_TAG, _out);
#elif PLATFORM_WINDOWS || PLATFORM_WINRT || PLATFORM_XBOX360 || PLATFORM_XBOXONE
    OutputDebugStringA(_out);
#elif PLATFORM_IOS || PLATFORM_OSX
#    if defined(__OBJC__)
    NSLog(@"%s", _out);
#    else
    NSLog(__CFStringMakeConstantString("%s"), _out);
#    endif // defined(__OBJC__)
#elif PLATFORM_EMSCRIPTEN
    emscripten_log(EM_LOG_CONSOLE, "%s", _out);
#else
    fputs(_out, stdout);
    fflush(stdout);
#endif // PLATFORM_
}

void debug_printf_vargs(const char* _format, va_list _argList)
{
    char temp[8192];
    char* out = temp;
    int32_t len = vsnprintf_(out, sizeof(temp), _format, _argList);
    if ( (int32_t)sizeof(temp) < len)
    {
        out = (char*)alloca(len+1);
        len = vsnprintf_(out, len, _format, _argList);
    }
    out[len] = '\0';
    debug_output(out);
}

void debug_printf(const char* _format, ...)
{
#if defined(DEBUG) || defined(_DEBUG)
    va_list argList;
    va_start(argList, _format);
    debug_printf_vargs(_format, argList);
    va_end(argList);
#endif
}

#define DBG_ADDRESS "%" PRIxPTR

void debug_printf_data(const void* _data, uint32_t _size, const char* _format, ...)
{
#define HEX_DUMP_WIDTH 16
#define HEX_DUMP_SPACE_WIDTH 48
#define HEX_DUMP_FORMAT "%-" STRINGIZE(HEX_DUMP_SPACE_WIDTH) "." STRINGIZE(HEX_DUMP_SPACE_WIDTH) "s"
    
    va_list argList;
    va_start(argList, _format);
    debug_printf_vargs(_format, argList);
    va_end(argList);
    
    debug_printf("\ndata: " DBG_ADDRESS ", size: %d\n", _data, _size);
    
    if (NULL != _data)
    {
        const uint8_t* data = (const uint8_t*)(_data);
        char hex[HEX_DUMP_WIDTH*3+1];
        char ascii[HEX_DUMP_WIDTH+1];
        uint32_t hexPos = 0;
        uint32_t asciiPos = 0;
        for (uint32_t ii = 0; ii < _size; ++ii)
        {
            snprintf_(&hex[hexPos], sizeof(hex)-hexPos, "%02x ", data[asciiPos]);
            hexPos += 3;
            
            ascii[asciiPos] = is_print(data[asciiPos]) ? data[asciiPos] : '.';
            asciiPos++;
            
            if (HEX_DUMP_WIDTH == asciiPos)
            {
                ascii[asciiPos] = '\0';
                debug_printf("\t" DBG_ADDRESS "\t" HEX_DUMP_FORMAT "\t%s\n", data, hex, ascii);
                data += asciiPos;
                hexPos = 0;
                asciiPos = 0;
            }
        }
        
        if (0 != asciiPos)
        {
            ascii[asciiPos] = '\0';
            debug_printf("\t" DBG_ADDRESS "\t" HEX_DUMP_FORMAT "\t%s\n", data, hex, ascii);
        }
    }
    
#undef HEX_DUMP_WIDTH
#undef HEX_DUMP_SPACE_WIDTH
#undef HEX_DUMP_FORMAT
}
