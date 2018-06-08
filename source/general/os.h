#ifndef os_h
#define os_h

#include "allocator.h"

BEGIN_DECLS

typedef enum _file_type
{
    FILE_TYPE_NONE = 0,
    FILE_TYPE_REGULAR,
    FILE_TYPE_DIRECTORY,
    FILE_TYPE_LAST = FILE_TYPE_DIRECTORY
} file_type;

typedef struct _file_info
{
    uint64_t size;
    file_type type;
    
} file_info;

void os_sleep(uint32_t _ms);

void os_yield();

uint32_t os_getTid();

size_t os_getProcessMemoryUsed();

void* os_dlopen(const char* _filePath);

void os_dlclose(void* _handle);

void* os_dlsym(void* _handle, const char* _symbol);

///
bool os_getenv(const char* _name, char* _out, uint32_t* _inOutSize);

///
void os_setenv(const char* _name, const char* _value);

///
void os_unsetenv(const char* _name);

///
int os_chdir(const char* _path);

///
char* os_pwd(char* _buffer, uint32_t _size);

///
bool os_getTempPath(char* _out, uint32_t* _inOutSize);

///
bool os_stat(const char* _filePath, file_info _fileInfo);

///
void* os_exec(const char* const* _argv);

END_DECLS

#endif /* os_h */
