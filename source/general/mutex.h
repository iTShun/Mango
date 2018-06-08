#ifndef mutex_h
#define mutex_h

#include "allocator.h"

BEGIN_DECLS

typedef struct _mutex
{
	BX_ALIGN_DECL(16, uint8_t) _internal[64];
} mutex;

void mutex_init(mutex *_self);
void mutex_lock(mutex *_self);
void mutex_unlock(mutex *_self);
void mutex_shutdown(mutex *_self);

END_DECLS

#endif /* mutex_h */