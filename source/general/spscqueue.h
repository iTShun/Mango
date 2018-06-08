#ifndef spscqueue_h
#define spscqueue_h

#include "semaphore.h"

#if BX_COMPILER_MSVC
#	include <Windows.h>
#else

#endif

BEGIN_DECLS

typedef struct _SpScUnboundedQueue_Node
{
	void* ptr;
	struct _SpScUnboundedQueue_Node* next;
} SpScNode;

typedef struct _SpScUnboundedQueue
{
	t_allocator* _allocator;
	SpScNode* first;
	SpScNode* divider;
	SpScNode* last;
} SpScUnboundedQueue;

#if BX_CONFIG_SUPPORTS_THREADING

typedef struct _SpScBlockingUnboundedQueue
{
	semaphore count;
	SpScUnboundedQueue queue;
} SpScBlockingUnboundedQueue;

#endif


static inline void* atomicExchangePtr(void** _ptr, void* _new)
{
#if BX_COMPILER_MSVC
	return InterlockedExchangePointer(_ptr, _new);
#else
	return __sync_lock_test_and_set(_ptr, _new);
#endif // BX_COMPILER
}

static inline void spsc_init(SpScUnboundedQueue *_self, t_allocator* _allocator)
{
	_self->_allocator = _allocator;
	_self->first = (SpScNode*)BX_ALLOC(_self->_allocator, sizeof(SpScNode));
	_self->divider = _self->first;
	_self->last = _self->first;
}

static inline void spsc_push(SpScUnboundedQueue *_self, void *_ptr)
{
	_self->last->next = (SpScNode*)BX_ALLOC(_self->_allocator, sizeof(SpScNode));
	_self->last->next->ptr = _ptr;
	atomicExchangePtr((void**)&_self->last, _self->last->next);
	while (_self->first != _self->divider)
	{
		SpScNode* node = _self->first;
		_self->first = _self->first->next;
		BX_SAFEFREE(_self->_allocator, node);
	}
}

static inline void* spsc_peek(SpScUnboundedQueue *_self)
{
	if (_self->divider != _self->last)
	{
		return _self->divider->next->ptr;
	}

	return NULL;
}

static inline void* spsc_pop(SpScUnboundedQueue *_self)
{
	if (_self->divider != _self->last)
	{
		void* ptr = _self->divider->next->ptr;
		atomicExchangePtr((void**)&_self->divider, _self->divider->next);
		return ptr;
	}

	return NULL;
}

static inline void spsc_shutdown(SpScUnboundedQueue *_self)
{
	while (NULL != _self->first)
	{
		SpScNode* node = _self->first;
		_self->first = node->next;
		BX_SAFEFREE(_self->_allocator, node);
	}
}


#if BX_CONFIG_SUPPORTS_THREADING

static inline void spscb_init(SpScBlockingUnboundedQueue *_self, t_allocator* _allocator)
{
	semaphore_init(&_self->count);
	spsc_init(&_self->queue, _allocator);
}

static inline void spscb_push(SpScBlockingUnboundedQueue *_self, void *_ptr)
{
	spsc_push(&_self->queue, _ptr);
	semaphore_post(&_self->count, -1);
}

static inline void* spscb_peek(SpScBlockingUnboundedQueue *_self)
{
	return spsc_peek(&_self->queue);
}

static inline void* spscb_pop(SpScBlockingUnboundedQueue *_self, int32_t _msecs)
{
	if (semaphore_wait(&_self->count, _msecs))
	{
		return spsc_pop(&_self->queue);
	}

	return NULL;
}

static inline void spscb_shutdown(SpScBlockingUnboundedQueue *_self)
{
	spsc_shutdown(&_self->queue);
	semaphore_shutdown(&_self->count);
}

#endif

END_DECLS

#endif /* spscqueue_h */