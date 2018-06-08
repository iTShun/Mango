#ifndef mpscqueue_h
#define mpscqueue_h

#include "allocator.h"
#include "mutex.h"
#include "spscqueue.h"

BEGIN_DECLS

typedef struct _MpScUnboundedQueue
{
	mutex _write;
	SpScUnboundedQueue queue;
} MpScUnboundedQueue;

typedef struct _MpScUnboundedBlockingQueue
{
	MpScUnboundedQueue queue;
	semaphore sem;
} MpScUnboundedBlockingQueue;


static inline void mpsc_init(MpScUnboundedQueue *_self, t_allocator* _allocator)
{
	mutex_init(&_self->_write);
	spsc_init(&_self->queue, _allocator);
}

static inline void mpsc_push(MpScUnboundedQueue *_self, void *_ptr)
{
	mutex_lock(&_self->_write);
	spsc_push(&_self->queue, _ptr);
	mutex_unlock(&_self->_write);
}

static inline void* mpsc_peek(MpScUnboundedQueue *_self)
{
	return spsc_peek(&_self->queue);
}

static inline void* mpsc_pop(MpScUnboundedQueue *_self)
{
	return spsc_pop(&_self->queue);
}

static inline void mpsc_shutdown(MpScUnboundedQueue *_self)
{
	spsc_shutdown(&_self->queue);
	mutex_shutdown(&_self->_write);
}


static inline void mpscb_init(MpScUnboundedBlockingQueue *_self, t_allocator* _allocator)
{
	mpsc_init(&_self->queue, _allocator);
}

static inline void mpscb_push(MpScUnboundedBlockingQueue *_self, void *_ptr)
{
	mpsc_push(&_self->queue, _ptr);
	semaphore_post(&_self->sem, -1);
}

static inline void* mpscb_pop(MpScUnboundedBlockingQueue *_self)
{
	semaphore_wait(&_self->sem, -1);
	return mpsc_pop(&_self->queue);
}

static inline void mpscb_shutdown(MpScUnboundedBlockingQueue *_self)
{
	mpsc_shutdown(&_self->queue);
}

END_DECLS

#endif /* mpscqueue_h */