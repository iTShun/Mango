#ifndef window_windows_h
#define window_windows_h

#include "base/ref.h"

#if BX_PLATFORM_WINDOWS

#include <Windows.h>

BEGIN_DECLS

#ifndef CONFIG_MAX_WINDOWS
#	define CONFIG_MAX_WINDOWS 1
#endif // CONFIG_MAX_WINDOWS

static int32_t WINDOW_DEFAULT_WIDTH = 1280;
static int32_t WINDOW_DEFAULT_HEIGHT = 720;

#undef  OBJECT
#define OBJECT window

/* Object interface */
OBJECT_INTERFACE

INHERIT_MEMBERS_OF(ref);
HWND private(hwnd)[CONFIG_MAX_WINDOWS];

OBJECT_METHODS

INHERIT_METHODS_OF(ref);

ENDOF_INTERFACE

/* Class interface */
CLASS_INTERFACE

t_window*const classMethod(create);
int32_t method_(run)
	int argc, char **argv __;
bool method(init);
void* classMethod(poll_event);
bool method_(dispatch_event)
void* event __;

ENDOF_INTERFACE

END_DECLS

#endif /* BX_PLATFORM_WINDOWS */
#endif /* window_windows_h */