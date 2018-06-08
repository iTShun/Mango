#define IMPLEMENTATION

#include "array.h"

#ifndef SIZE_MAX
#   define SIZE_MAX ((size_t) -1)
#endif

#define ARRAY_TYPE_IS_VALID(t)  \
    (t > ARRAY_TYPE_NONE && t <= ARRAY_TYPE_MAX)

static size_t sizeof_type[ARRAY_TYPE_MAX + 1] =
{
    0,                          /* ARRAY_TYPE_NONE */
    sizeof(bool),
    sizeof(char),
    sizeof(unsigned char),
    sizeof(short),
    sizeof(unsigned short),
    sizeof(int),
    sizeof(unsigned int),
    sizeof(long),
    sizeof(unsigned long),
    sizeof(float),
    sizeof(double),
    sizeof(void *)
};

#define MAX_SIZE(da)  (SIZE_MAX / sizeof_type[da->m.type])

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
    BX_SAFEFREE(getDefaultAllocator(), this->m.base);
}

t_array
classMethodDecl_(*const create)
    array_type _type, size_t _size __
{
    t_array *this = array.alloc();
    if (this)
    {
        assert(ARRAY_TYPE_IS_VALID(_type));
        
        size_t max_size;
        
        this->m.type = _type;
        
        if (_size == 0)
            this->m.size = ARRAY_MAX_SIZE;
        else if (_size > (max_size = MAX_SIZE(this)))
            this->m.size = max_size;
        else
            this->m.size = _size;
        
        this->m.base = BX_ALLOC(getDefaultAllocator(), this->m.size * sizeof_type[this->m.type]);
        
        assert(this->m.base != NULL);
    }
    else
        BX_SAFECLASS(this);
        
    return this;
}

bool
methodDecl_(resize)
    size_t _size __
{
    do
    {
        void *new_base = NULL;
        size_t new_size, max_size;
        
        assert(this != NULL);
        
        if (_size < this->m.size || _size >= (max_size = MAX_SIZE(this)) - 1)
            break;
        
        new_size = ((max_size - ARRAY_RESIZE_PAD - 1) >= _size) ?
        _size + ARRAY_RESIZE_PAD + 1 : max_size;
        
        new_base = BX_REALLOC(getDefaultAllocator(), this->m.base, new_size * sizeof_type[this->m.type]);
        assert(new_base != NULL);
        
        this->m.base = new_base;
        this->m.size = new_size;
        
        return true;
    }while(false);
    
    return false;
}

size_t 
methodDecl(size)
{
	return this->m.size;
}

#define ARRAY_GETSET_F(pfx, dtype, ctype)    \
bool methodDecl_(set##pfx) size_t idx, ctype v __                   \
{                                                                   \
    ctype *ep;                                                      \
                                                                    \
    assert (this != NULL);                                          \
                                                                    \
    /* be strict over what we're assigning */                       \
    assert (this->m.type == dtype);                                 \
                                                                    \
    /* silently handle resize in case an element is set past the    \
    * actual index range */                                         \
    if (idx > this->m.size - 1)                                     \
        assert (array.resize(this, idx));                           \
                                                                    \
    /* get the selected slot */                                     \
    ep = (ctype *) this->m.base + idx;                              \
                                                                    \
    if (ep == NULL) return false;                                   \
                                                                    \
    /* assign */                                                    \
    *ep = v;                                                        \
                                                                    \
    return true;                                                    \
}                                                                   \
                                                                    \
ctype methodDecl_(get##pfx) size_t idx __                           \
{                                                                   \
    ctype *ep;                                                      \
                                                                    \
    assert (this != NULL);                                          \
    /* be strict over what we're returning */                       \
    assert (this->m.type == dtype);                                 \
                                                                    \
    /* check overflow */                                            \
    assert (idx <= this->m.size - 1);                               \
                                                                    \
    ep = (ctype *) this->m.base + idx;                              \
                                                                    \
    if (ep == NULL) return (ctype)0;                                \
                                                                    \
    return *ep;                                                     \
}

ARRAY_GETSET_F(Bool, ARRAY_TYPE_BOOL, bool)
ARRAY_GETSET_F(Char, ARRAY_TYPE_CHAR, char)
ARRAY_GETSET_F(UChar, ARRAY_TYPE_UCHAR, unsigned char)
ARRAY_GETSET_F(Short, ARRAY_TYPE_SHORT, short)
ARRAY_GETSET_F(UShort, ARRAY_TYPE_USHORT, unsigned short)
ARRAY_GETSET_F(Int, ARRAY_TYPE_INT, int)
ARRAY_GETSET_F(UInt, ARRAY_TYPE_UINT, unsigned int)
ARRAY_GETSET_F(Long, ARRAY_TYPE_LONG, long)
ARRAY_GETSET_F(ULong, ARRAY_TYPE_ULONG, unsigned long)
ARRAY_GETSET_F(Float, ARRAY_TYPE_FLOAT, float)
ARRAY_GETSET_F(Double, ARRAY_TYPE_DOUBLE, double)

void
methodDecl_(*const setPtr)
    size_t idx, void *v __
{
    void **ep, *old_ep;
    ep = old_ep = NULL;
    
    assert(this != NULL);
    assert(this->m.type == ARRAY_TYPE_PTR);
    
    if (idx > this->m.size - 1)
        assert (array.resize(this, idx));
        
    ep = (void **) this->m.base + idx;
    if (ep == NULL) return NULL;
    /* save old value (could be NULL) */
    old_ep = *ep;
    
    /* overwrite with the supplied value */
    *ep = v;
        
    return old_ep;
}

void
methodDecl_(*const getPtr)
    size_t idx __
{
    assert(this != NULL);
    assert(this->m.type == ARRAY_TYPE_PTR);
    
    assert (idx <= this->m.size - 1);
        
    return *((void **) this->m.base + idx);
}

CLASS_IMPLEMENTATION

methodName(create),
methodName(resize),
methodName(size),
methodName(setBool),
methodName(getBool),
methodName(setChar),
methodName(getChar),
methodName(setUChar),
methodName(getUChar),
methodName(setShort),
methodName(getShort),
methodName(setUShort),
methodName(getUShort),
methodName(setInt),
methodName(getInt),
methodName(setUInt),
methodName(getUInt),
methodName(setLong),
methodName(getLong),
methodName(setULong),
methodName(getULong),
methodName(setFloat),
methodName(getFloat),
methodName(setDouble),
methodName(getDouble),
methodName(setPtr),
methodName(getPtr)

ENDOF_IMPLEMENTATION

