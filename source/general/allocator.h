#ifndef allocator_h
#define allocator_h

#include "platform/platform.h"
#include "oopc/ooc.h"

#if BX_PLATFORM_BSD
#   if defined(__GLIBC__)
#       include_next <alloca.h>
#   endif
#elif BX_PLATFORM_IOS || BX_PLATFORM_OSX
#   include <malloc/malloc.h>
#   if BX_PLATFORM_OSX
#       include <alloca.h>
#   endif
#elif BX_CRT_MINGW
#   include <malloc.h>
#elif BX_CRT_MSVC
#   include <malloc.h>
#	define alloca _alloca
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <float.h>
#include <assert.h>

BEGIN_DECLS

#define BX_ALIGN_MASK(_value, _mask) ( ( (_value)+(_mask) ) & ( (~0)&(~(_mask) ) ) )
#define BX_ALIGN_16(_value) BX_ALIGN_MASK(_value, 0xf)
#define BX_ALIGN_256(_value) BX_ALIGN_MASK(_value, 0xff)
#define BX_ALIGN_4096(_value) BX_ALIGN_MASK(_value, 0xfff)

#if defined(DEBUG) || defined(_DEBUG)
#   define _FILE_LINE_      __FILE__, __LINE__
#else
#   define _FILE_LINE_      NULL, NULL
#endif

#define BX_ALLOC(_allocator, _size)                         allocator.allocate(_allocator, _size, 0, _FILE_LINE_)
#define BX_REALLOC(_allocator, _ptr, _size)                 allocator.realloc(_allocator, _ptr, _size, 0, _FILE_LINE_)
#define BX_FREE(_allocator, _ptr)                           allocator.free(_allocator, _ptr, 0, _FILE_LINE_)
#define BX_SAFEFREE(_allocator, _ptr)                       if (_ptr) { allocator.free(_allocator, _ptr, 0, _FILE_LINE_); _ptr = NULL; }
#define BX_ALIGNED_ALLOC(_allocator, _size, _align)         allocator.allocate(_allocator, _size, _align, _FILE_LINE_)
#define BX_ALIGNED_REALLOC(_allocator, _ptr, _size, _align) allocator.realloc(_allocator, _ptr, _size, _align, _FILE_LINE_)
#define BX_ALIGNED_FREE(_allocator, _ptr, _align)           allocator.free(_allocator, _ptr, _align, _FILE_LINE_)
#define BX_ALIGNED_SAFEFREE(_allocator, _ptr, _align)       if (_ptr) { allocator.free(_allocator, _ptr, _align, _FILE_LINE_); _ptr = NULL; }


#ifndef BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT
#    define BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT 8
#endif // BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT

#undef  OBJECT
#define OBJECT allocator

/* Object interface */
BASEOBJECT_INTERFACE

char const* private(name);

BASEOBJECT_METHODS

ENDOF_INTERFACE

/* Class interface */
CLASS_INTERFACE

t_allocator* const classMethod_(create)
    char* const _name __;

bool classMethod_(isAligned)
    const void* _ptr, size_t _align __;

void* classMethod_(alignPtr)
    void* _ptr, size_t _extra, size_t _align __;

void* method_(allocate)
    size_t _size, size_t _align, const char* _file, uint32_t _line __;

void method_(free)
    void* _ptr, size_t _align, const char* _file, uint32_t _line __;

void* method_(realloc)
    void* _ptr, size_t _size, size_t _align, const char* _file, uint32_t _line __;

void* method_(alignedAlloc)
    size_t _size, size_t _align, const char* _file, uint32_t _line __;

void method_(alignedFree)
    void* _ptr, size_t _align, const char* _file, uint32_t _line __;

void* method_(alignedRealloc)
    void* _ptr, size_t _size, size_t _align, const char* _file, uint32_t _line __;

ENDOF_INTERFACE

extern t_allocator *g_defaultAllocator;
extern t_allocator* getDefaultAllocator();

///
extern void memCopy(void* _dst, const void* _src, size_t _numBytes);

///
extern void memCopy_(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _srcPitch, uint32_t _dstPitch);

///
extern void gather(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _srcPitch);

///
extern void scatter(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _dstPitch);

///
extern void memMove(void* _dst, const void* _src, size_t _numBytes);

///
extern void memSet(void* _dst, uint8_t _ch, size_t _numBytes);

///
extern int32_t memCmp(const void* _lhs, const void* _rhs, size_t _numBytes);

END_DECLS

#endif /* allocator_h */
