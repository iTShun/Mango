#include "semaphore.h"
#include "debug.h"

#if BX_CONFIG_SUPPORTS_THREADING

#if BX_PLATFORM_OSX \
||  BX_PLATFORM_IOS
#	include <dispatch/dispatch.h>
#elif BX_PLATFORM_POSIX
#	include <errno.h>
#	include <pthread.h>
#	include <semaphore.h>
#	include <time.h>
#elif  BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_WINRT   \
	|| BX_PLATFORM_XBOXONE
#	include <windows.h>
#	include <limits.h>
#	if BX_PLATFORM_XBOXONE
#		include <synchapi.h>
#	endif // BX_PLATFORM_XBOXONE
#endif // BX_PLATFORM_

typedef struct _semaphore_internal
{
#if BX_PLATFORM_OSX \
||  BX_PLATFORM_IOS
	dispatch_semaphore_t m_handle;
#elif BX_PLATFORM_POSIX
	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;
	int32_t m_count;
#elif  BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_WINRT   \
	|| BX_PLATFORM_XBOXONE
	HANDLE m_handle;
#endif // BX_PLATFORM_
} SemaphoreInternal;

#if BX_PLATFORM_OSX \
||  BX_PLATFORM_IOS

bool semaphore_init(semaphore *self)
{
	SemaphoreInternal* si = (SemaphoreInternal*)self->_internal;

	si->m_handle = dispatch_semaphore_create(0);
	check(NULL != si->m_handle, "dispatch_semaphore_create failed.");
	return true;
}

void semaphore_post(semaphore *self, uint32_t count)
{
	SemaphoreInternal* si = (SemaphoreInternal*)self->_internal;

	for (uint32_t ii = 0; ii < count; ++ii)
	{
		dispatch_semaphore_signal(si->m_handle);
	}
}

bool semaphore_wait(semaphore *self, int32_t msecs)
{
	SemaphoreInternal* si = (SemaphoreInternal*)self->_internal;

	dispatch_time_t dt = 0 > msecs
		? DISPATCH_TIME_FOREVER
		: dispatch_time(DISPATCH_TIME_NOW, msecs * 1000000)
		;
	return !dispatch_semaphore_wait(si->m_handle, dt);
}

void semaphore_shutdown(semaphore *self)
{
	SemaphoreInternal* si = (SemaphoreInternal*)self->_internal;

	dispatch_release(si->m_handle);
}

#elif BX_PLATFORM_POSIX

static uint64_t toNs(const timespec& _ts)
{
	return _ts.tv_sec*(UINT64_C)(1000000000) + _ts.tv_nsec;
}

static void toTimespecNs(timespec& _ts, uint64_t _nsecs)
{
	_ts.tv_sec = _nsecs / (UINT64_C)(1000000000);
	_ts.tv_nsec = _nsecs%(UINT64_C)(1000000000);
}

static void toTimespecMs(timespec& _ts, int32_t _msecs)
{
	toTimespecNs(_ts, _msecs * 1000000);
}

static void add(timespec& _ts, int32_t _msecs)
{
	uint64_t ns = toNs(_ts);
	toTimespecNs(_ts, ns + (uint64_t)(_msecs) * 1000000);
}

bool semaphore_init(semaphore *self)
{
	SemaphoreInternal* si = (SemaphoreInternal*)self->_internal;

	si->m_count = 0;

	int result;

	result = pthread_mutex_init(&si->m_mutex, NULL);
	check(0 == result, "pthread_mutex_init %d", result);

	result = pthread_cond_init(&si->m_cond, NULL);
	check(0 == result, "pthread_cond_init %d", result);

	return true;
}

void semaphore_post(semaphore *self, uint32_t count)
{
	SemaphoreInternal* si = (SemaphoreInternal*)self->_internal;

	int result = pthread_mutex_lock(&si->m_mutex);
	check(0 == result, "pthread_mutex_lock %d", result);

	for (uint32_t ii = 0; ii < count; ++ii)
	{
		result = pthread_cond_signal(&si->m_cond);
		BX_CHECK(0 == result, "pthread_cond_signal %d", result);
	}

	si->m_count += count;

	result = pthread_mutex_unlock(&si->m_mutex);
	check(0 == result, "pthread_mutex_unlock %d", result);
}

bool semaphore_wait(semaphore *self, int32_t msecs)
{
	SemaphoreInternal* si = (SemaphoreInternal*)self->_internal;

	int result = pthread_mutex_lock(&si->m_mutex);
	check(0 == result, "pthread_mutex_lock %d", result);

	if (-1 == msecs)
	{
		while (0 == result
			&& 0 >= si->m_count)
		{
			result = pthread_cond_wait(&si->m_cond, &si->m_mutex);
		}
	}
	else
	{
		timespec ts;
		clock_gettime(CLOCK_REALTIME, &ts);
		add(ts, msecs);

		while (0 == result
			&& 0 >= si->m_count)
		{
			result = pthread_cond_timedwait(&si->m_cond, &si->m_mutex, &ts);
		}
		}

	bool ok = 0 == result;

	if (ok)
	{
		--si->m_count;
	}

	result = pthread_mutex_unlock(&si->m_mutex);
	check(0 == result, "pthread_mutex_unlock %d", result);

	return ok;
}

void semaphore_shutdown(semaphore *self)
{
	SemaphoreInternal* si = (SemaphoreInternal*)self->_internal;

	int result;
	result = pthread_cond_destroy(&si->m_cond);
	check(0 == result, "pthread_cond_destroy %d", result);

	result = pthread_mutex_destroy(&si->m_mutex);
	check(0 == result, "pthread_mutex_destroy %d", result);
}

#elif  BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_WINRT   \
	|| BX_PLATFORM_XBOXONE

bool semaphore_init(semaphore *self)
{
	SemaphoreInternal* si = (SemaphoreInternal*)self->_internal;

#if BX_PLATFORM_WINRT \
||  BX_PLATFORM_XBOXONE
	si->m_handle = CreateSemaphoreExW(NULL, 0, LONG_MAX, NULL, 0, SEMAPHORE_ALL_ACCESS);
#else
	si->m_handle = CreateSemaphoreA(NULL, 0, LONG_MAX, NULL);
#endif
	check(NULL != si->m_handle, "Failed to create Semaphore!");

	return true;
}

void semaphore_post(semaphore *self, uint32_t count)
{
	SemaphoreInternal* si = (SemaphoreInternal*)self->_internal;

	ReleaseSemaphore(si->m_handle, count, NULL);
}

bool semaphore_wait(semaphore *self, int32_t msecs)
{
	SemaphoreInternal* si = (SemaphoreInternal*)self->_internal;

	DWORD milliseconds = (0 > msecs) ? INFINITE : msecs;
#if BX_PLATFORM_WINRT \
||  BX_PLATFORM_XBOXONE
	return WAIT_OBJECT_0 == WaitForSingleObjectEx(si->m_handle, milliseconds, FALSE);
#else
	return WAIT_OBJECT_0 == WaitForSingleObject(si->m_handle, milliseconds);
#endif
}

void semaphore_shutdown(semaphore *self)
{
	SemaphoreInternal* si = (SemaphoreInternal*)self->_internal;

	CloseHandle(si->m_handle);
}

#endif // BX_PLATFORM_

#endif /* BX_CONFIG_SUPPORTS_THREADING */