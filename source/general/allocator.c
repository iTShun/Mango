#define IMPLEMENTATION

#include "allocator.h"

t_allocator *g_defaultAllocator = NULL;

t_allocator* getDefaultAllocator()
{
    if (g_defaultAllocator == NULL)
        g_defaultAllocator = allocator.create("default");
    
    return g_defaultAllocator;
}

/*
 ---------------------
 Object implementation
 ---------------------
 */

BASEOBJECT_IMPLEMENTATION

ENDOF_IMPLEMENTATION

/*
 --------------------
 Class implementation
 --------------------
 */

initClassDecl() /* required */
{
    
}

dtorDecl() /* required */
{
    free((void*)this->m.name);
    this->m.name = NULL;
}

t_allocator
classMethodDecl_(*const create)
    char* const _name __
{
    t_allocator *const this = allocator.alloc();
    if (this)
    {
        this->m.name = strdup(_name);
    }
    return this;
}

bool
classMethodDecl_(isAligned)
    const void* _ptr, size_t _align __
{
    union { const void* ptr; uintptr_t addr; } un;
    un.ptr = _ptr;
    return 0 == (un.addr & (_align-1) );
}

void
classMethodDecl_(*const alignPtr)
    void* _ptr, size_t _extra, size_t _align __
{
    union { void* ptr; uintptr_t addr; } un;
    un.ptr = _ptr;
    uintptr_t unaligned = un.addr + _extra; // space for header
    uintptr_t mask = _align-1;
    uintptr_t aligned = BX_ALIGN_MASK(unaligned, mask);
    un.addr = aligned;
    return un.ptr;
}

void
methodDecl_(*const allocate)
    size_t _size, size_t _align, const char* _file, uint32_t _line __
{
    return allocator.realloc(this, NULL, _size, _align, _file, _line);
}

void
methodDecl_(free)
    void* _ptr, size_t _align, const char* _file, uint32_t _line __
{
    allocator.realloc(this, _ptr, 0, _align, _file, _line);
}

void
methodDecl_(*const realloc)
    void* _ptr, size_t _size, size_t _align, const char* _file, uint32_t _line __
{
	void* result = NULL;
    if (_size == 0)
    {
        if (_ptr != NULL)
        {
            if (_align <= BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT)
            {
                free(_ptr);
                return NULL;
            }
            
#    if BX_COMPILER_MSVC
            _aligned_free(_ptr);
#    else
            allocator.alignedFree(this, _ptr, _align, _file, _line);
#    endif
        }
        return NULL;
    }
    else if (_ptr == NULL)
    {
        if (_align <= BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT)
        {
			result = malloc(_size);
			memSet(result, 0, _size);
			return result;
        }
        
#    if BX_COMPILER_MSVC
		result = _aligned_malloc(_size, _align);
		memSet(result, 0, _size);
		return result;
#    else
		result = allocator.alignedAlloc(this, _size, _align, _file, _line);
		memSet(result, 0, _size);
		return result;
#    endif
    }
    
    if (BX_CONFIG_ALLOCATOR_NATURAL_ALIGNMENT >= _align)
    {
		result = realloc(_ptr, _size);
		memSet(result, 0, _size);
		return result;
    }
    
#    if BX_COMPILER_MSVC
	result = _aligned_realloc(_ptr, _size, _align);
#    else
	result = allocator.alignedRealloc(this, _ptr, _size, _align, _file, _line);
#    endif
	memSet(result, 0, _size);
	return result;
}

void
methodDecl_(*const alignedAlloc)
    size_t _size, size_t _align, const char* _file, uint32_t _line __
{
    size_t total = _size + _align;
    uint8_t* ptr = (uint8_t*)allocator.allocate(this, total, 0, _file, _line);
    uint8_t* aligned = (uint8_t*)allocator.alignPtr(ptr, sizeof(uint32_t), _align);
    uint32_t* header = (uint32_t*)aligned - 1;
    *header = (uint32_t)(aligned - ptr);
    return aligned;
}

void
methodDecl_(alignedFree)
    void* _ptr, size_t _align, const char* _file, uint32_t _line __
{
    uint8_t* aligned = (uint8_t*)_ptr;
    uint32_t* header = (uint32_t*)aligned - 1;
    uint8_t* ptr = aligned - *header;
    allocator.free(this, ptr, 0, _file, _line);
}

void
methodDecl_(*const alignedRealloc)
    void* _ptr, size_t _size, size_t _align, const char* _file, uint32_t _line __
{
    if (_ptr == NULL)
    {
        return allocator.alignedAlloc(this, _size, _align, _file, _line);
    }
    
    uint8_t* aligned = (uint8_t*)_ptr;
    uint32_t offset = *( (uint32_t*)aligned - 1);
    uint8_t* ptr = aligned - offset;
    size_t total = _size + _align;
    ptr = (uint8_t*)allocator.realloc(this, ptr, total, 0, _file, _line);
    uint8_t* newAligned = (uint8_t*)allocator.alignPtr(ptr, sizeof(uint32_t), _align);
    
    if (newAligned == aligned)
    {
        return aligned;
    }
    
    aligned = ptr + offset;
    memMove(newAligned, aligned, _size);
    uint32_t* header = (uint32_t*)newAligned - 1;
    *header = (uint32_t)(newAligned - ptr);
    return newAligned;
}

CLASS_IMPLEMENTATION

methodName(create),
methodName(isAligned),
methodName(alignPtr),
methodName(allocate),
methodName(free),
methodName(realloc),
methodName(alignedAlloc),
methodName(alignedFree),
methodName(alignedRealloc)

ENDOF_IMPLEMENTATION

void memCopyRef(void* _dst, const void* _src, size_t _numBytes)
{
    uint8_t* dst = (uint8_t*)_dst;
    const uint8_t* end = dst + _numBytes;
    const uint8_t* src = (const uint8_t*)_src;
    while (dst != end)
    {
        *dst++ = *src++;
    }
}

void memCopy(void* _dst, const void* _src, size_t _numBytes)
{
#if BX_CRT_NONE
    memCopyRef(_dst, _src, _numBytes);
#else
    memcpy(_dst, _src, _numBytes);
#endif // BX_CRT_NONE
}

void memCopy_(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _srcPitch, uint32_t _dstPitch)
{
    const uint8_t* src = (const uint8_t*)_src;
    uint8_t* dst = (uint8_t*)_dst;
    
    for (uint32_t ii = 0; ii < _num; ++ii)
    {
        memCopy(dst, src, _size);
        src += _srcPitch;
        dst += _dstPitch;
    }
}

///
void gather(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _srcPitch)
{
    memCopy_(_dst, _src, _size, _num, _srcPitch, _size);
}

///
void scatter(void* _dst, const void* _src, uint32_t _size, uint32_t _num, uint32_t _dstPitch)
{
    memCopy_(_dst, _src, _size, _num, _size, _dstPitch);
}

void memMoveRef(void* _dst, const void* _src, size_t _numBytes)
{
    uint8_t* dst = (uint8_t*)_dst;
    const uint8_t* src = (const uint8_t*)_src;
    
    if (_numBytes == 0
        ||  dst == src)
    {
        return;
    }
    
    //    if (src+_numBytes <= dst || end <= src)
    if (dst < src)
    {
        memCopy(_dst, _src, _numBytes);
        return;
    }
    
    for (intptr_t ii = _numBytes-1; ii >= 0; --ii)
    {
        dst[ii] = src[ii];
    }
}

void memMove(void* _dst, const void* _src, size_t _numBytes)
{
#if BX_CRT_NONE
    memMoveRef(_dst, _src, _numBytes);
#else
    memmove(_dst, _src, _numBytes);
#endif // BX_CRT_NONE
}

void memSetRef(void* _dst, uint8_t _ch, size_t _numBytes)
{
    uint8_t* dst = (uint8_t*)_dst;
    const uint8_t* end = dst + _numBytes;
    while (dst != end)
    {
        *dst++ = (char)(_ch);
    }
}

void memSet(void* _dst, uint8_t _ch, size_t _numBytes)
{
#if BX_CRT_NONE
    memSetRef(_dst, _ch, _numBytes);
#else
    memset(_dst, _ch, _numBytes);
#endif // BX_CRT_NONE
}

int32_t memCmpRef(const void* _lhs, const void* _rhs, size_t _numBytes)
{
    const char* lhs = (const char*)_lhs;
    const char* rhs = (const char*)_rhs;
    for (
         ; 0 < _numBytes && *lhs == *rhs
         ; ++lhs, ++rhs, --_numBytes
         )
    {
    }
    
    return 0 == _numBytes ? 0 : *lhs - *rhs;
}

int32_t memCmp(const void* _lhs, const void* _rhs, size_t _numBytes)
{
#if BX_CRT_NONE
    return memCmpRef(_lhs, _rhs, _numBytes);
#else
    return memcmp(_lhs, _rhs, _numBytes);
#endif // BX_CRT_NONE
}
