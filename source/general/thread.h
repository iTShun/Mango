#ifndef thread_h
#define thread_h

#include "allocator.h"
#include "mpscqueue.h"

BEGIN_DECLS

typedef int32_t(*ThreadFn)(void* _self, void* _userData);

#undef  OBJECT
#define OBJECT thread

/* Object interface */
BASEOBJECT_INTERFACE

BX_ALIGN_DECL(16, uint8_t) _internal[64];
ThreadFn func;
void* userData;
MpScUnboundedBlockingQueue queue;
semaphore sem;
uint32_t stackSize;
int32_t exitCode;
bool running;

BASEOBJECT_METHODS

int32_t constMethod(entry);

ENDOF_INTERFACE

/* Class interface */
CLASS_INTERFACE

t_thread *const classMethod_(create)
	ThreadFn _func, void *_userData __;
t_thread *const classMethod_(createFull)
	ThreadFn _func, void *_userData, uint32_t _stackSize, const char* _name __;
void method(shutdown);
void method_(setName)
	const char* _name __;
void method_(push)
	void* _ptr __;
void* method(pop);

ENDOF_INTERFACE

typedef struct _TlsData
{
	BX_ALIGN_DECL(16, uint8_t) _internal[64];
} TlsData;

extern void __thread_init__(t_thread* _th, const char* _name);
extern int32_t __thread_entry__(t_thread* _th);
extern void __thread_shutdown__(t_thread* _th);
extern void __thread_setThreadName__(t_thread* _th, const char* _name);

extern void tlsdata_init(TlsData* _data);
extern void tlsdata_set(TlsData* _data, void *_ptr);
extern void* tlsdata_get(TlsData* _data);
extern void tlsdata_shutdown(TlsData* _data);

END_DECLS

#endif /* thread_h */
