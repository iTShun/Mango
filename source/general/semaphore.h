#ifndef semaphore_h
#define semaphore_h

#include "allocator.h"

BEGIN_DECLS

typedef struct _semaphore
{
	BX_ALIGN_DECL(16, uint8_t) _internal[128];
} semaphore;

bool semaphore_init(semaphore *_self);
void semaphore_post(semaphore *_self, uint32_t _count);
bool semaphore_wait(semaphore *_self, int32_t _msecs);
void semaphore_shutdown(semaphore *_self);

END_DECLS

#endif /* semaphore_h */