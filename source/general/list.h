#ifndef list_h
#define list_h

#include "allocator.h"

BEGIN_DECLS

typedef struct _list_head
{
    struct _list_item *first;
    struct _list_item **last;
} list_head;

typedef struct _list_item
{
    struct _list_item **prev;
    struct _list_item *next;
    void *ptr;
} list_item;

#undef  OBJECT
#define OBJECT list

/* Object interface */
BASEOBJECT_INTERFACE


list_head private(head);
size_t private(count);

BASEOBJECT_METHODS

ENDOF_INTERFACE

/* Class interface */
CLASS_INTERFACE

t_list *const classMethod(create);
bool method(clear);
bool method_(add) void *_ptr __;
bool method_(insert) size_t _idx, void *_ptr __;
bool method_(del) void *_ptr __;
bool method_(delWithIndex) size_t _idx, void *_ptr __;
void* method_(get) size_t _idx __;
size_t method(count);
void* method_(first) void **_it __;
void* method_(next) void **_it __;

ENDOF_INTERFACE

END_DECLS

#endif /* list_h */
