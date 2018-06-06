#ifndef list_h
#define list_h

#include "allocator.h"

BEGIN_DECLS

#ifndef TAILQ_HEAD
#   define TAILQ_HEAD(name, type)                                       \
struct name {                                                           \
    struct type *tqh_first;     /* first element */                     \
    struct type **tqh_last;     /* addr of last next element */         \
}
#endif

#ifndef TAILQ_ENTRY
#   define TAILQ_ENTRY(type)                                            \
struct {                                                                \
    struct type *tqe_next;      /* next element */                      \
    struct type **tqe_prev;     /* address of previous next element */  \
}
#endif

typedef struct _list_item
{
    TAILQ_ENTRY(_list_item) np;
    void *ptr;
} list_item;

#undef  OBJECT
#define OBJECT list

/* Object interface */
BASEOBJECT_INTERFACE

TAILQ_HEAD(_list_head, _list_item) private(head);
size_t private(count);

BASEOBJECT_METHODS

ENDOF_INTERFACE

/* Class interface */
CLASS_INTERFACE

t_list *const classMethod(create);
bool method(clear);
bool method_(add) void *ptr __;
bool method_(insert) void *ptr __;
bool method_(del) void *ptr __;
bool method_(delWithIndex) size_t idx, void *ptr __;
void* method_(get) size_t idx __;
size_t method(count);
void* method_(first) void **it __;
void* method_(next) void **it __;

ENDOF_INTERFACE

END_DECLS

#endif /* list_h */
