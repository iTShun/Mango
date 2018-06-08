#ifndef window_h
#define window_h

#include "platform.h"

#if BX_PLATFORM_ANDROID

#elif BX_PLATFORM_EMSCRIPTEN

#elif BX_PLATFORM_IOS

#elif BX_PLATFORM_OSX
#   include "window-osx.h"
#elif BX_PLATFORM_WINDOWS
#   include "window-windows.h"
#	if BX_CRT_MSVC
#		pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )
#	endif
#elif BX_PLATFORM_WINRT || BX_PLATFORM_XBOXONE

#elif BX_PLATFORM_BSD || BX_PLATFORM_LINUX || BX_PLATFORM_RPI

#endif


#endif /* window_h */
