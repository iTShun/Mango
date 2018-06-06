#ifndef ref_h
#define ref_h

#include "platform/platform.h"
#include "oopc/ooc.h"
#include "debug.h"
#include <stdint.h>
#include <stdbool.h>

BEGIN_DECLS

#define safe_free(p) if (p) { free(p); p = NULL; }
#define safe_delete(obj) if (obj) { delete(obj); obj = NULL; }
#define safe_release(obj) if (obj && ref.release(obj)) { obj = NULL; }

#undef  OBJECT
#define OBJECT ref

/* Object interface */
BASEOBJECT_INTERFACE

char* private(name);
int32_t private(ref_count);

BASEOBJECT_METHODS

void constMethod(print);

ENDOF_INTERFACE

/* Class interface */
CLASS_INTERFACE

t_ref* classMethod_(create) char* name __;
void method_(init) char* name __;
void method_(copy) t_ref* per __;
int32_t method(getRefCount);
void method(retain);
bool method(release);

ENDOF_INTERFACE

END_DECLS

#endif /* ref_h */
