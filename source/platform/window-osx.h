#ifndef window_osx_h
#define window_osx_h

#include "base/ref.h"

#if PLATFORM_OSX

BEGIN_DECLS

static int32_t WINDOW_DEFAULT_WIDTH = 1280;
static int32_t WINDOW_DEFAULT_HEIGHT = 720;

#undef  OBJECT
#define OBJECT window

/* Object interface */
OBJECT_INTERFACE

INHERIT_MEMBERS_OF(ref);
void* private(handle);
uint32_t private(origin_x);
uint32_t private(origin_y);
uint32_t private(width);
uint32_t private(height);

int32_t private(style);
bool private(fullscreen);
bool private(exit);

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

#endif /* PLATFORM_OSX */
#endif /* window_osx_h */
