#ifndef array_h
#define array_h

#include "allocator.h"

BEGIN_DECLS

#define ARRAY_MAX_SIZE      512
#define ARRAY_RESIZE_PAD    100

#define ARRAY_FOREACH(_index, _array) \
	for (int32_t _index = 0; _index < array.size(_array); ++_index)

typedef enum
{
    ARRAY_TYPE_NONE = 0,
    ARRAY_TYPE_BOOL,
    ARRAY_TYPE_CHAR,
    ARRAY_TYPE_UCHAR,
    ARRAY_TYPE_SHORT,
    ARRAY_TYPE_USHORT,
    ARRAY_TYPE_INT,
    ARRAY_TYPE_UINT,
    ARRAY_TYPE_LONG,
    ARRAY_TYPE_ULONG,
    ARRAY_TYPE_FLOAT,
    ARRAY_TYPE_DOUBLE,
    ARRAY_TYPE_PTR,
    ARRAY_TYPE_MAX = ARRAY_TYPE_PTR
} array_type;

#undef  OBJECT
#define OBJECT array

/* Object interface */
BASEOBJECT_INTERFACE

array_type private(type);
size_t private(size);
void* private(base);

BASEOBJECT_METHODS

ENDOF_INTERFACE

/* Class interface */
CLASS_INTERFACE

t_array* const classMethod_(create)
    array_type _type, size_t _size __;
bool method_(resize)
    size_t _size __;
size_t method(size);

bool method_(setBool)
    size_t idx, bool v __;
bool method_(getBool)
    size_t idx __;

bool method_(setChar)
    size_t idx, char v __;
char method_(getChar)
    size_t idx __;

bool method_(setUChar)
    size_t idx, unsigned char v __;
unsigned char method_(getUChar)
    size_t idx __;

bool method_(setShort)
    size_t idx, short v __;
short method_(getShort)
    size_t idx __;

bool method_(setUShort)
    size_t idx, unsigned short v __;
unsigned short method_(getUShort)
    size_t idx __;

bool method_(setInt)
    size_t idx, int v __;
int method_(getInt)
    size_t idx __;

bool method_(setUInt)
    size_t idx, unsigned int v __;
unsigned int method_(getUInt)
    size_t idx __;

bool method_(setLong)
    size_t idx, long v __;
long method_(getLong)
    size_t idx __;

bool method_(setULong)
    size_t idx, unsigned long v __;
unsigned long method_(getULong)
    size_t idx __;

bool method_(setFloat)
    size_t idx, float v __;
float method_(getFloat)
    size_t idx __;

bool method_(setDouble)
    size_t idx, double v __;
double method_(getDouble)
    size_t idx __;

void* method_(setPtr)
    size_t idx, void *v __;
void* method_(getPtr)
    size_t idx __;

ENDOF_INTERFACE


END_DECLS

#endif /* array_h */
