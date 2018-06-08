/*
 * Copyright 2010-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_SPSCQUEUE_H_HEADER_GUARD
#define BX_SPSCQUEUE_H_HEADER_GUARD

#include "allocator.h"
#include "semaphore.h"

#if BX_COMPILER_MSVC
#	include <windows.h>
#endif

namespace bx
{

	static inline void* atomicExchangePtr(void** _ptr, void* _new)
	{
#if BX_COMPILER_MSVC
		return InterlockedExchangePointer(_ptr, _new);
#else
		return __sync_lock_test_and_set(_ptr, _new);
#endif // BX_COMPILER
	}

	///
	class SpScUnboundedQueue
	{
	public:
		///
		SpScUnboundedQueue(t_allocator* _allocator);

		///
		~SpScUnboundedQueue();

		///
		void push(void* _ptr);

		///
		void* peek();

		///
		void* pop();

	private:
		struct Node
		{
			///
			Node(void* _ptr);

			void* m_ptr;
			Node* m_next;
		};

		t_allocator* m_allocator;
		Node* m_first;
		Node* m_divider;
		Node* m_last;
	};

	///
	template<typename Ty>
	class SpScUnboundedQueueT
	{
	public:
		///
		SpScUnboundedQueueT(t_allocator* _allocator);

		///
		~SpScUnboundedQueueT();

		///
		void push(Ty* _ptr);

		///
		Ty* peek();

		///
		Ty* pop();

	private:
		SpScUnboundedQueue m_queue;
	};

#if BX_CONFIG_SUPPORTS_THREADING
	///
	class SpScBlockingUnboundedQueue
	{
	public:
		///
		SpScBlockingUnboundedQueue(t_allocator* _allocator);

		///
		~SpScBlockingUnboundedQueue();

		///
		void push(void* _ptr); // producer only

		///
		void* peek(); // consumer only

		///
		void* pop(int32_t _msecs = -1); // consumer only

	private:
		Semaphore m_count;
		SpScUnboundedQueue m_queue;
	};

	///
	template<typename Ty>
	class SpScBlockingUnboundedQueueT
	{
	public:
		///
		SpScBlockingUnboundedQueueT(t_allocator* _allocator);

		///
		~SpScBlockingUnboundedQueueT();

		///
		void push(Ty* _ptr); // producer only

		///
		Ty* peek(); // consumer only

		///
		Ty* pop(int32_t _msecs = -1); // consumer only

	private:
		SpScBlockingUnboundedQueue m_queue;
	};
#endif // BX_CONFIG_SUPPORTS_THREADING

} // namespace bx

#include "spscqueue.inl"

#endif // BX_SPSCQUEUE_H_HEADER_GUARD
