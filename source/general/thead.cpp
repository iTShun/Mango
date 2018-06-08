#include "thread.h"
#include "debug.h"

#if    BX_PLATFORM_ANDROID \
	|| BX_PLATFORM_LINUX   \
	|| BX_PLATFORM_IOS     \
	|| BX_PLATFORM_OSX     \
	|| BX_PLATFORM_PS4     \
	|| BX_PLATFORM_RPI
#	include <pthread.h>
#	if defined(__FreeBSD__)
#		include <pthread_np.h>
#	endif
#	if BX_PLATFORM_LINUX && (BX_CRT_GLIBC < 21200)
#		include <sys/prctl.h>
#	endif // BX_PLATFORM_
#elif  BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_WINRT   \
	|| BX_PLATFORM_XBOXONE
#	include <windows.h>
#	include <limits.h>
#	include <errno.h>
#	if BX_PLATFORM_WINRT
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
#	endif // BX_PLATFORM_WINRT
#endif // BX_PLATFORM_

#if BX_CONFIG_SUPPORTS_THREADING

struct ThreadInternal
{
#if    BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_WINRT   \
	|| BX_PLATFORM_XBOXONE
	static DWORD WINAPI threadFunc(LPVOID _arg);
	HANDLE m_handle;
	DWORD  m_threadId;
#elif BX_PLATFORM_POSIX
	static void* threadFunc(void* _arg);
	pthread_t m_handle;
#endif // BX_PLATFORM_
};

struct TlsDataInternal
{
#if BX_PLATFORM_WINDOWS
	uint32_t m_id;
#elif !(BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT)
	pthread_key_t m_id;
#endif // BX_PLATFORM_*
};

#if    BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_XBOXONE \
	|| BX_PLATFORM_WINRT
DWORD WINAPI ThreadInternal::threadFunc(LPVOID _arg)
{
	t_thread* th = (t_thread*)_arg;
	int32_t result = sendMsg(th, entry);
	return result;
}
#else
void* ThreadInternal::threadFunc(void* _arg)
{
	t_thread* th = (t_thread*)_arg;
	union
	{
		void* ptr;
		int32_t i;
	} cast;
	cast.i = sendMsg(th, entry);
	return cast.ptr;
}
#endif // BX_PLATFORM_

void __thread_init__(t_thread* th, const char* name)
{
	assert(th != NULL);

	ThreadInternal* ti = (ThreadInternal*)th->m._internal;
#if    BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_WINRT   \
	|| BX_PLATFORM_XBOXONE
	ti->m_handle = INVALID_HANDLE_VALUE;
	ti->m_threadId = UINT32_MAX;
#elif BX_PLATFORM_POSIX
	ti->m_handle = 0;
#endif // BX_PLATFORM_

	th->m.running = true;

#if    BX_PLATFORM_WINDOWS \
	|| BX_PLATFORM_XBOXONE
	ti->m_handle = ::CreateThread(NULL
		, th->m.stackSize
		, (LPTHREAD_START_ROUTINE)ti->threadFunc
		, th
		, 0
		, NULL
	);
#elif BX_PLATFORM_WINRT
	ti->m_handle = CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
	auto workItemHandler = ref new WorkItemHandler([=](IAsyncAction^)
	{
		m_exitCode = ti->threadFunc(th);
		SetEvent(ti->m_handle);
	}
		, CallbackContext::Any
		);

	ThreadPool::RunAsync(workItemHandler, WorkItemPriority::Normal, WorkItemOptions::TimeSliced);
#elif BX_PLATFORM_POSIX
	int result;

	pthread_attr_t attr;
	result = pthread_attr_init(&attr);
	check(0 == result, "pthread_attr_init failed! %d", result);

	if (0 != th->m.stackSize)
	{
		result = pthread_attr_setstacksize(&attr, th->m.stackSize);
		check(0 == result, "pthread_attr_setstacksize failed! %d", result);
	}

	result = pthread_create(&ti->m_handle, &attr, &ti->threadFunc, th);
	check(0 == result, "pthread_attr_setschedparam failed! %d", result);
#else
#	error "Not implemented!"
#endif // BX_PLATFORM_

	semaphore_wait(&th->m.sem, -1);

	if (NULL != name)
	{
		__thread_setThreadName__(th, name);
	}
}

int32_t __thread_entry__(t_thread* th)
{
	assert(th != NULL);

#if BX_PLATFORM_WINDOWS
	ThreadInternal* ti = (ThreadInternal*)th->m._internal;
	ti->m_threadId = ::GetCurrentThreadId();
#endif // BX_PLATFORM_WINDOWS

	semaphore_post(&th->m.sem, -1);
	int32_t result = th->m.func(th, th->m.userData);
	return result;
}

void __thread_shutdown__(t_thread* th)
{
	check(th->m.running, "Not running!");
	ThreadInternal* ti = (ThreadInternal*)th->m._internal;
#if BX_PLATFORM_WINDOWS
	WaitForSingleObject(ti->m_handle, INFINITE);
	GetExitCodeThread(ti->m_handle, (DWORD*)&th->m.exitCode);
	CloseHandle(ti->m_handle);
	ti->m_handle = INVALID_HANDLE_VALUE;
#elif BX_PLATFORM_WINRT || BX_PLATFORM_XBOXONE
	WaitForSingleObjectEx(ti->m_handle, INFINITE, FALSE);
	CloseHandle(ti->m_handle);
	ti->m_handle = INVALID_HANDLE_VALUE;
#elif BX_PLATFORM_POSIX
	union
	{
		void* ptr;
		int32_t i;
	} cast;
	pthread_join(ti->m_handle, &cast.ptr);
	th->m.exitCode = cast.i;
	ti->m_handle = 0;
#endif // BX_PLATFORM_

	th->m.running = false;
}

void __thread_setThreadName__(t_thread* th, const char* name)
{
	assert(th != NULL);

	ThreadInternal* ti = (ThreadInternal*)th->m._internal;
#if BX_PLATFORM_OSX || BX_PLATFORM_IOS
	pthread_setname_np(name);
#elif (BX_CRT_GLIBC >= 21200) && ! BX_PLATFORM_HURD
	pthread_setname_np(ti->m_handle, name);
#elif BX_PLATFORM_LINUX
	prctl(PR_SET_NAME, name, 0, 0, 0);
#elif BX_PLATFORM_BSD
#	ifdef __NetBSD__
	pthread_setname_np(ti->m_handle, "%s", (void*)name);
#	else
	pthread_set_name_np(ti->m_handle, name);
#	endif // __NetBSD__
#elif BX_PLATFORM_WINDOWS && BX_COMPILER_MSVC
#	pragma pack(push, 8)
	struct ThreadName
	{
		DWORD  type;
		LPCSTR name;
		DWORD  id;
		DWORD  flags;
	};
#	pragma pack(pop)
	ThreadName tn;
	tn.type = 0x1000;
	tn.name = name;
	tn.id = ti->m_threadId;
	tn.flags = 0;

	__try
	{
		RaiseException(0x406d1388
			, 0
			, sizeof(tn) / 4
			, reinterpret_cast<ULONG_PTR*>(&tn)
		);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
#else
	
#endif // BX_PLATFORM_
}


#if BX_PLATFORM_WINDOWS

void tlsdata_init(TlsData* data)
{
	TlsDataInternal* ti = (TlsDataInternal*)data->_internal;
	ti->m_id = TlsAlloc();
	check(TLS_OUT_OF_INDEXES != ti->m_id, "Failed to allocated TLS index (err: 0x%08x).", GetLastError());
}

void tlsdata_set(TlsData* data, void *ptr)
{
	TlsDataInternal* ti = (TlsDataInternal*)data->_internal;

	TlsSetValue(ti->m_id, ptr);
}

void* tlsdata_get(TlsData* data)
{
	TlsDataInternal* ti = (TlsDataInternal*)data->_internal;

	return TlsGetValue(ti->m_id);
}

void tlsdata_shutdown(TlsData* data)
{
	TlsDataInternal* ti = (TlsDataInternal*)data->_internal;

	BOOL result = TlsFree(ti->m_id);
	check(0 != result, "Failed to free TLS index (err: 0x%08x).", GetLastError());
}


#elif !(BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT)

void tlsdata_init(TlsData* data)
{
	TlsDataInternal* ti = (TlsDataInternal*)data->_internal;

	int result = pthread_key_create(&ti->m_id, NULL);
	check(0 == result, "pthread_key_create failed %d.", result);
}

void tlsdata_set(TlsData* data, void *ptr)
{
	TlsDataInternal* ti = (TlsDataInternal*)data->_internal;

	int result = pthread_setspecific(ti->m_id, ptr);
	check(0 == result, "pthread_setspecific failed %d.", result);
}

void* tlsdata_get(TlsData* data)
{
	TlsDataInternal* ti = (TlsDataInternal*)data->_internal;

	return pthread_getspecific(ti->m_id);
}

void tlsdata_shutdown(TlsData* data)
{
	TlsDataInternal* ti = (TlsDataInternal*)data->_internal;

	int result = pthread_key_delete(ti->m_id);
	check(0 == result, "pthread_key_delete failed %d.", result);
}

#endif // BX_PLATFORM_*

#endif /* BX_CONFIG_SUPPORTS_THREADING */