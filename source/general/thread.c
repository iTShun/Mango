#define IMPLEMENTATION

#include "thread.h"

/*
---------------------
Object implementation
---------------------
*/
int32_t
constMethodDecl(entry)
{
	//return __thread_entry__(this);
	return 32;
}

BASEOBJECT_IMPLEMENTATION

methodName(entry)

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
	if (this->m.running)
		thread.shutdown(this);
}

t_thread
classMethodDecl_(*const create)
	ThreadFn func, void *userData __
{
	return thread.createFull(func, userData, 0, NULL);
}
t_thread
classMethodDecl_(*const createFull)
	ThreadFn func, void *userData, uint32_t stackSize, const char* name __
{
	t_thread *this = thread.alloc();
	if (this)
	{
		this->m.func = func;
		this->m.userData = userData;
		this->m.stackSize = stackSize;
		this->m.exitCode = 0;
		this->m.running = false;

		mpscb_init(&this->m.queue, getDefaultAllocator());
		semaphore_init(&this->m.sem);
		__thread_init__(this, name);
	}
	else
		BX_SAFECLASS(this);

	return this;
}

void 
methodDecl(shutdown)
{
	__thread_shutdown__(this);
	semaphore_shutdown(&this->m.sem);
	mpscb_shutdown(&this->m.queue);
}

void 
methodDecl_(setName)
	const char* name __
{
	__thread_setThreadName__(this, name);
}

void 
methodDecl_(push)
	void* ptr __
{
	mpscb_push(&this->m.queue, ptr);
}

void
methodDecl(*pop)
{
	return mpscb_pop(&this->m.queue);
}

CLASS_IMPLEMENTATION

methodName(create),
methodName(createFull),
methodName(shutdown),
methodName(setName),
methodName(push),
methodName(pop)

ENDOF_IMPLEMENTATION