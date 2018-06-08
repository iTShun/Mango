#include "os.h"

#if !BX_PLATFORM_NONE

#include <stdio.h>
#include <sys/stat.h>
#include "debug.h"
#include "string.h"
#include "uint32_t.h"

#if BX_PLATFORM_WINDOWS || BX_PLATFORM_WINRT
#    include <windows.h>
#    include <psapi.h>
#	 pragma comment(lib, "psapi.lib")
#elif  BX_PLATFORM_ANDROID \
|| BX_PLATFORM_EMSCRIPTEN \
|| BX_PLATFORM_BSD \
|| BX_PLATFORM_HURD \
|| BX_PLATFORM_IOS \
|| BX_PLATFORM_LINUX \
|| BX_PLATFORM_NACL \
|| BX_PLATFORM_OSX \
|| BX_PLATFORM_PS4 \
|| BX_PLATFORM_RPI \
|| BX_PLATFORM_STEAMLINK
#    include <sched.h> // sched_yield
#    if BX_PLATFORM_BSD \
|| BX_PLATFORM_IOS \
|| BX_PLATFORM_NACL \
|| BX_PLATFORM_OSX \
|| BX_PLATFORM_PS4 \
|| BX_PLATFORM_STEAMLINK
#        include <pthread.h> // mach_port_t
#    endif // BX_PLATFORM_*

#    include <time.h> // nanosleep
#    if !BX_PLATFORM_PS4 && !BX_PLATFORM_NACL
#        include <dlfcn.h> // dlopen, dlclose, dlsym
#    endif // !BX_PLATFORM_PS4 && !BX_PLATFORM_NACL

#    if BX_PLATFORM_ANDROID
#        include <malloc.h> // mallinfo
#    elif   BX_PLATFORM_LINUX \
|| BX_PLATFORM_RPI \
|| BX_PLATFORM_STEAMLINK
#        include <unistd.h> // syscall
#        include <sys/syscall.h>
#    elif BX_PLATFORM_OSX
#        include <mach/mach.h> // mach_task_basic_info
#    elif BX_PLATFORM_HURD
#        include <pthread/pthread.h> // pthread_self
#    elif BX_PLATFORM_ANDROID
#        include "debug.h" // getTid is not implemented...
#    endif // BX_PLATFORM_ANDROID
#endif // BX_PLATFORM_

#if BX_CRT_MSVC
#    include <direct.h> // _getcwd
#else
#    include <unistd.h> // getcwd
#endif // BX_CRT_MSVC

void os_sleep(uint32_t _ms)
{
#if BX_PLATFORM_WINDOWS || BX_PLATFORM_XBOX360
    Sleep(_ms);
#elif BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT
    debug_output("sleep is not implemented"); debug_break();
#else
    struct timespec req = {(time_t)_ms/1000, (long)((_ms%1000)*1000000)};
    struct timespec rem = {0, 0};
    nanosleep(&req, &rem);
#endif // BX_PLATFORM_
}

void os_yield()
{
#if BX_PLATFORM_WINDOWS
    SwitchToThread();
#elif BX_PLATFORM_XBOX360
    Sleep(0);
#elif BX_PLATFORM_XBOXONE || BX_PLATFORM_WINRT
    debug_output("yield is not implemented"); debug_break();
#else
    sched_yield();
#endif // BX_PLATFORM_
}

uint32_t os_getTid()
{
#if BX_PLATFORM_WINDOWS
    return GetCurrentThreadId();
#elif BX_PLATFORM_LINUX || BX_PLATFORM_RPI || BX_PLATFORM_STEAMLINK
    return (pid_t)syscall(SYS_gettid);
#elif BX_PLATFORM_IOS || BX_PLATFORM_OSX
    return (mach_port_t)pthread_mach_thread_np(pthread_self() );
#elif BX_PLATFORM_BSD || BX_PLATFORM_NACL
    // Casting __nc_basic_thread_data*... need better way to do this.
    return *(uint32_t*)pthread_self();
#elif BX_PLATFORM_HURD
    return (pthread_t)pthread_self();
#else
    //#    pragma message "not implemented."
    debug_output("getTid is not implemented"); debug_break();
    return 0;
#endif //
}

size_t os_getProcessMemoryUsed()
{
#if BX_PLATFORM_ANDROID
    struct mallinfo mi = mallinfo();
    return mi.uordblks;
#elif BX_PLATFORM_LINUX || BX_PLATFORM_HURD
    FILE* file = fopen("/proc/self/statm", "r");
    if (NULL == file)
    {
        return 0;
    }
    
    long pages = 0;
    int items = fscanf(file, "%*s%ld", &pages);
    fclose(file);
    return 1 == items
    ? pages * sysconf(_SC_PAGESIZE)
    : 0
    ;
#elif BX_PLATFORM_OSX
#    if defined(MACH_TASK_BASIC_INFO)
    struct mach_task_basic_info info;
    mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
    
    int const result = task_info(mach_task_self()
                                 , MACH_TASK_BASIC_INFO
                                 , (task_info_t)&info
                                 , &infoCount
                                 );
#    else // MACH_TASK_BASIC_INFO
    task_basic_info info;
    mach_msg_type_number_t infoCount = TASK_BASIC_INFO_COUNT;
    
    int const result = task_info(mach_task_self()
                                 , TASK_BASIC_INFO
                                 , (task_info_t)&info
                                 , &infoCount
                                 );
#    endif // MACH_TASK_BASIC_INFO
    if (KERN_SUCCESS != result)
    {
        return 0;
    }
    
    return info.resident_size;
#elif BX_PLATFORM_WINDOWS
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess()
                         , &pmc
                         , sizeof(pmc)
                         );
    return pmc.WorkingSetSize;
#else
    return 0;
#endif // BX_PLATFORM_*
}

void* os_dlopen(const char* _filePath)
{
#if BX_PLATFORM_WINDOWS
    return (void*)LoadLibraryA(_filePath);
#elif  BX_PLATFORM_EMSCRIPTEN \
|| BX_PLATFORM_NACL \
|| BX_PLATFORM_PS4 \
|| BX_PLATFORM_XBOXONE \
|| BX_PLATFORM_WINRT
    return NULL;
#else
    return dlopen(_filePath, RTLD_LOCAL|RTLD_LAZY);
#endif // BX_PLATFORM_
}

void os_dlclose(void* _handle)
{
#if BX_PLATFORM_WINDOWS
    FreeLibrary( (HMODULE)_handle);
#elif  BX_PLATFORM_EMSCRIPTEN \
|| BX_PLATFORM_NACL \
|| BX_PLATFORM_PS4 \
|| BX_PLATFORM_XBOXONE \
|| BX_PLATFORM_WINRT
    
#else
    dlclose(_handle);
#endif // BX_PLATFORM_
}

void* os_dlsym(void* _handle, const char* _symbol)
{
#if BX_PLATFORM_WINDOWS
    return (void*)GetProcAddress( (HMODULE)_handle, _symbol);
#elif  BX_PLATFORM_EMSCRIPTEN \
|| BX_PLATFORM_NACL \
|| BX_PLATFORM_PS4 \
|| BX_PLATFORM_XBOXONE \
|| BX_PLATFORM_WINRT
    return NULL;
#else
    return dlsym(_handle, _symbol);
#endif // BX_PLATFORM_
}

bool os_getenv(const char* _name, char* _out, uint32_t* _inOutSize)
{
#if BX_PLATFORM_WINDOWS
    DWORD len = GetEnvironmentVariableA(_name, _out, *_inOutSize);
    bool result = len != 0 && len < *_inOutSize;
    *_inOutSize = len;
    return result;
#elif  BX_PLATFORM_PS4 \
|| BX_PLATFORM_XBOXONE \
|| BX_PLATFORM_WINRT
    return false;
#else
    const char* ptr = getenv(_name);
    uint32_t len = 0;
    bool result = false;
    if (NULL != ptr)
    {
        len = (uint32_t)strLen(ptr, INT32_MAX);
        
        result = len != 0 && len < *_inOutSize;
        if (len < *_inOutSize)
        {
            strCopy(_out, len, ptr, INT32_MAX);
        }
    }
    
    *_inOutSize = len;
    return result;
#endif // BX_PLATFORM_
}

void os_setenv(const char* _name, const char* _value)
{
#if BX_PLATFORM_WINDOWS
    SetEnvironmentVariableA(_name, _value);
#elif  BX_PLATFORM_PS4 \
|| BX_PLATFORM_XBOXONE \
|| BX_PLATFORM_WINRT
    
#else
    setenv(_name, _value, 1);
#endif // BX_PLATFORM_
}

void os_unsetenv(const char* _name)
{
#if BX_PLATFORM_WINDOWS
    SetEnvironmentVariableA(_name, NULL);
#elif  BX_PLATFORM_PS4 \
|| BX_PLATFORM_XBOXONE \
|| BX_PLATFORM_WINRT
    
#else
    unsetenv(_name);
#endif // BX_PLATFORM_
}

int os_chdir(const char* _path)
{
#if BX_PLATFORM_PS4 \
|| BX_PLATFORM_XBOXONE \
|| BX_PLATFORM_WINRT
    return -1;
#elif BX_CRT_MSVC
    return _chdir(_path);
#else
    return chdir(_path);
#endif // BX_COMPILER_
}

char* os_pwd(char* _buffer, uint32_t _size)
{
#if BX_PLATFORM_PS4 \
|| BX_PLATFORM_XBOXONE \
|| BX_PLATFORM_WINRT
    return NULL;
#elif BX_CRT_MSVC
    return _getcwd(_buffer, (int)_size);
#else
    return getcwd(_buffer, _size);
#endif // BX_COMPILER_
}

bool os_getTempPath(char* _out, uint32_t* _inOutSize)
{
#if BX_PLATFORM_WINDOWS
    uint32_t len = GetTempPathA(*_inOutSize, _out);
    bool result = len != 0 && len < *_inOutSize;
    *_inOutSize = len;
    return result;
#else
    static const char* s_tmp[] =
    {
        "TMPDIR",
        "TMP",
        "TEMP",
        "TEMPDIR",
        
        NULL
    };
    
    for (const char** tmp = s_tmp; *tmp != NULL; ++tmp)
    {
        uint32_t len = *_inOutSize;
        *_out = '\0';
        bool result = os_getenv(*tmp, _out, &len);
        
        if (result
            &&  len != 0
            &&  len < *_inOutSize)
        {
            *_inOutSize = len;
            return result;
        }
    }
    
    file_info fi;
    if (os_stat("/tmp", fi)
        &&  FILE_TYPE_DIRECTORY == fi.type)
    {
        strCopy(_out, *_inOutSize, "/tmp", INT32_MAX);
        *_inOutSize = 4;
        return true;
    }
    
    return false;
#endif // BX_PLATFORM_*
}

bool os_stat(const char* _filePath, file_info _fileInfo)
{
    _fileInfo.size = 0;
    _fileInfo.type = FILE_TYPE_NONE;
    
#if BX_COMPILER_MSVC
    struct _stat64 st;
    int32_t result = _stat64(_filePath, &st);
    
    if (0 != result)
    {
        return false;
    }
    
    if (0 != (st.st_mode & _S_IFREG) )
    {
        _fileInfo.type = FILE_TYPE_REGULAR;
    }
    else if (0 != (st.st_mode & _S_IFDIR) )
    {
        _fileInfo.type = FILE_TYPE_DIRECTORY;
    }
#else
    struct stat st;
    int32_t result = stat(_filePath, &st);
    if (0 != result)
    {
        return false;
    }
    
    if (0 != (st.st_mode & S_IFREG) )
    {
        _fileInfo.type = FILE_TYPE_REGULAR;
    }
    else if (0 != (st.st_mode & S_IFDIR) )
    {
        _fileInfo.type = FILE_TYPE_DIRECTORY;
    }
#endif // BX_COMPILER_MSVC
    
    _fileInfo.size = st.st_size;
    
    return true;
}

void* os_exec(const char* const* _argv)
{
#if BX_PLATFORM_LINUX || BX_PLATFORM_HURD
    pid_t pid = fork();
    
    if (0 == pid)
    {
        int result = execvp(_argv[0], (char *const*)(&_argv[1]) );
        return NULL;
    }
    
    return (void*)(uintptr_t)(pid);
#elif BX_PLATFORM_WINDOWS
    STARTUPINFO si;
    memSet(&si, 0, sizeof(STARTUPINFO) );
    si.cb = sizeof(STARTUPINFO);
    
    PROCESS_INFORMATION pi;
    memSet(&pi, 0, sizeof(PROCESS_INFORMATION) );
    
    int32_t total = 0;
    for (uint32_t ii = 0; NULL != _argv[ii]; ++ii)
    {
        total += (int32_t)strLen(_argv[ii], INT32_MAX) + 1;
    }
    
    char* temp = (char*)alloca(total);
    int32_t len = 0;
    for(uint32_t ii = 0; NULL != _argv[ii]; ++ii)
    {
        len += snprintf_(&temp[len], uint32_imax(0, total-len)
                        , "%s "
                        , _argv[ii]
                        );
    }
    
    bool ok = !!CreateProcessA(_argv[0]
                               , temp
                               , NULL
                               , NULL
                               , false
                               , 0
                               , NULL
                               , NULL
                               , &si
                               , &pi
                               );
    if (ok)
    {
        return pi.hProcess;
    }
    
    return NULL;
#else
    return NULL;
#endif // BX_PLATFORM_LINUX || BX_PLATFORM_HURD
}


#endif // !BX_PLATFORM_NONE
