/*
 * Copyright 2010-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef BX_MPSCQUEUE_H_HEADER_GUARD
#define BX_MPSCQUEUE_H_HEADER_GUARD

#include "allocator.h"
#include "mutex.h"
#include "spscqueue.h"

namespace bx
{
	///
	template <typename Ty>
	class MpScUnboundedQueueT
	{
	public:
		///
		MpScUnboundedQueueT(t_allocator* _allocator);

		///
		~MpScUnboundedQueueT();

		///
		void push(Ty* _ptr); // producer only

		///
		Ty* peek(); // consumer only

		///
		Ty* pop(); // consumer only

	private:
		Mutex m_write;
		SpScUnboundedQueueT<Ty> m_queue;
	};

	///
	template <typename Ty>
	class MpScUnboundedBlockingQueue
	{
	public:
		///
		MpScUnboundedBlockingQueue(t_allocator* _allocator);

		///
		~MpScUnboundedBlockingQueue();

		///
		void push(Ty* _ptr); // producer only

		///
		Ty* pop(); // consumer only

	private:
		MpScUnboundedQueueT<Ty> m_queue;
		Semaphore m_sem;
	};

} // namespace bx

#include "mpscqueue.inl"

#endif // BX_MPSCQUEUE_H_HEADER_GUARD
