#define IMPLEMENTATION

#include "window-osx.h"

#if PLATFORM_OSX

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject<NSApplicationDelegate>
{
    bool terminated;
}

+ (AppDelegate *)sharedDelegate;
- (id)init;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
- (bool)applicationHasTerminated;

@end

@interface Window : NSObject<NSWindowDelegate>
{
    uint32_t windowCount;
}

+ (Window*)sharedDelegate;
- (id)init;
- (void)windowCreated:(NSWindow*)window;
- (void)windowWillClose:(NSNotification*)notification;
- (BOOL)windowShouldClose:(NSWindow*)window;
- (void)windowDidResize:(NSNotification*)notification;
- (void)windowDidBecomeKey:(NSNotification *)notification;
- (void)windowDidResignKey:(NSNotification *)notification;

@end

/////////////////////////////

/*
 ---------------------
 Object implementation
 ---------------------
 */

void
constMethodOvldDecl(print, ref)
{
    sendCMsg(this, ref, print);
    //printf("\tXX:\t%s\n", sub_cast(this, ref)->m.XX);
}

OBJECT_IMPLEMENTATION

SUPERCLASS(ref)

ENDOF_IMPLEMENTATION

/*
 --------------------
 Class implementation
 --------------------
 */

initClassDecl() /* required */
{
    /* initialize super class */
    initSuper(ref);
    
    /* overload super class methods */
    overload(ref.print) = methodOvldName(print, ref);
    
    debug_printf("window initClassDecl");
}

dtorDecl() /* required */
{
    ref._ref(super(this, ref));
    
    debug_printf("window dtorDecl");
}

t_window
classMethodDecl(*const create)
{
    t_window *const this = window.alloc();
    if (this) window.init(this);
    return this;
}

int32_t
methodDecl_(*const run) int argc, char **argv __
{
    [NSApplication sharedApplication];
    
    id dg = [AppDelegate sharedDelegate];
    [NSApp setDelegate:dg];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];
    [NSApp finishLaunching];
    
    [[NSNotificationCenter defaultCenter]
     postNotificationName:NSApplicationWillFinishLaunchingNotification
     object:NSApp];

    [[NSNotificationCenter defaultCenter]
     postNotificationName:NSApplicationDidFinishLaunchingNotification
     object:NSApp];

    id quitMenuItem = [NSMenuItem new];
    [quitMenuItem
     initWithTitle:@"Quit"
     action:@selector(terminate:)
     keyEquivalent:@"q"];

    id appMenu = [NSMenu new];
    [appMenu addItem:quitMenuItem];

    id appMenuItem = [NSMenuItem new];
    [appMenuItem setSubmenu:appMenu];

    id menubar = [[NSMenu new] autorelease];
    [menubar addItem:appMenuItem];
    [NSApp setMainMenu:menubar];

    this->m.style = 0
                    | NSTitledWindowMask
                    | NSClosableWindowMask
                    | NSMiniaturizableWindowMask
                    | NSResizableWindowMask;

    NSRect screenRect = [[NSScreen mainScreen] frame];
    const float centerX = (screenRect.size.width  - (float)WINDOW_DEFAULT_WIDTH) * 0.5f;
    const float centerY = (screenRect.size.height - (float)WINDOW_DEFAULT_HEIGHT) * 0.5f;

    NSRect rect = NSMakeRect(centerX, centerY, WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);
    NSWindow* wnd = [[NSWindow alloc]
                        initWithContentRect:rect
                        styleMask:this->m.style
                        backing:NSBackingStoreBuffered defer:NO
                        ];
    NSString* appName = [[NSProcessInfo processInfo] processName];
    [wnd setTitle:appName];
    [wnd makeKeyAndOrderFront:wnd];
    [wnd setAcceptsMouseMovedEvents:YES];
    [wnd setBackgroundColor:[NSColor blackColor]];
    [[Window sharedDelegate] windowCreated:wnd];
    
    NSRect frame = [wnd frame];
    
    this->m.handle = wnd;
    this->m.origin_x = frame.origin.x;
    this->m.origin_y = frame.origin.y;
    this->m.width = frame.size.width;
    this->m.height = frame.size.height;

    while (!(this->m.exit = [dg applicationHasTerminated]) )
    {
        @autoreleasepool
        {
//            if (bgfx::RenderFrame::Exiting == bgfx::renderFrame() )
//            {
//                break;
//            }
        }
        
        while (window.dispatch_event(this, window.poll_event()))
        {
            
        }
    }
    
    return 0;
}

bool
methodDecl(*const init)
{
    do
    {
        ref.init(super(this, ref), "window-osx");
        this->m.handle = NULL;
        this->m.origin_x = 0;
        this->m.origin_y = 0;
        this->m.width = 0;
        this->m.height = 0;
        
        this->m.style = 0;
        this->m.fullscreen = false;
        this->m.exit = false;
        
        return true;
    }while(false);
    
    return false;
}

void
classMethodDecl(*const poll_event)
{
    return [NSApp
            nextEventMatchingMask:NSAnyEventMask
            untilDate:[NSDate distantPast] // do not wait for event
            inMode:NSDefaultRunLoopMode
            dequeue:YES
            ];
}

bool
methodDecl_(*const dispatch_event)  void* _event __
{
    if (_event)
    {
        NSEvent* event = (NSEvent*)_event;
        NSEventType eventType = [event type];
        
        switch (eventType)
        {
            case NSMouseMoved:
            case NSLeftMouseDragged:
            case NSRightMouseDragged:
            case NSOtherMouseDragged:
            {
                //                getMousePos(&m_mx, &m_my);
                //                m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll);
                break;
            }
                
            case NSLeftMouseDown:
            {
                // Command + Left Mouse Button acts as middle! This just a temporary solution!
                // This is because the average OSX user doesn't have middle mouse click.
                //                MouseButton::Enum mb = ([event modifierFlags] & NSCommandKeyMask) ? MouseButton::Middle : MouseButton::Left;
                //                m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll, mb, true);
                break;
            }
                
            case NSLeftMouseUp:
            {
                //                m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll, MouseButton::Left, false);
                //                m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll, MouseButton::Middle, false); // TODO: remove!
                break;
            }
                
            case NSRightMouseDown:
            {
                //                m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll, MouseButton::Right, true);
                break;
            }
                
            case NSRightMouseUp:
            {
                //                m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll, MouseButton::Right, false);
                break;
            }
                
            case NSOtherMouseDown:
            {
                //                m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll, MouseButton::Middle, true);
                break;
            }
                
            case NSOtherMouseUp:
            {
                //                m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll, MouseButton::Middle, false);
                break;
            }
                
            case NSScrollWheel:
            {
                //                m_scrollf += [event deltaY];
                //
                //                m_scroll = (int32_t)m_scrollf;
                //                m_eventQueue.postMouseEvent(s_defaultWindow, m_mx, m_my, m_scroll);
                break;
            }
                
            case NSKeyDown:
            {
                //                uint8_t modifiers = 0;
                //                uint8_t pressedChar[4];
                //                Key::Enum key = handleKeyEvent(event, &modifiers, &pressedChar[0]);
                //
                //                // Returning false means that we take care of the key (instead of the default behavior)
                //                if (key != Key::None)
                //                {
                //                    if (key == Key::KeyQ && (modifiers & Modifier::RightMeta) )
                //                    {
                //                        m_eventQueue.postExitEvent();
                //                    }
                //                    else
                //                    {
                //                        enum { ShiftMask = Modifier::LeftShift|Modifier::RightShift };
                //                        m_eventQueue.postCharEvent(s_defaultWindow, 1, pressedChar);
                //                        m_eventQueue.postKeyEvent(s_defaultWindow, key, modifiers, true);
                //                        return false;
                //                    }
                //                }
                
                break;
            }
                
            case NSKeyUp:
            {
                //                uint8_t modifiers  = 0;
                //                uint8_t pressedChar[4];
                //                Key::Enum key = handleKeyEvent(event, &modifiers, &pressedChar[0]);
                //
                //                if (key != Key::None)
                //                {
                //                    m_eventQueue.postKeyEvent(s_defaultWindow, key, modifiers, false);
                //                    return false;
                //                }
                
                break;
            }
        }
        
        [NSApp sendEvent:event];
        [NSApp updateWindows];
        
        return true;
    }
    
    return false;
}

CLASS_IMPLEMENTATION

methodName(create),
methodName(run),
methodName(init),
methodName(poll_event),
methodName(dispatch_event)

ENDOF_IMPLEMENTATION


//////////////////////////////////////////////////

static id s_app_delegate;
static id s_window_delegate;

@implementation AppDelegate

+ (AppDelegate *)sharedDelegate
{
    if (s_app_delegate == nil)
        s_app_delegate = [AppDelegate new];
    
    return s_app_delegate;
}

- (id)init
{
    self = [super init];
    
    if (nil == self)
    {
        return nil;
    }
    
    self->terminated = false;
    return self;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    self->terminated = true;
    return NSTerminateCancel;
}

- (bool)applicationHasTerminated
{
    return self->terminated;
}

@end

@implementation Window

+ (Window*)sharedDelegate
{
    if (s_window_delegate == nil)
        s_window_delegate = [Window new];
    
    return s_window_delegate;
}

- (id)init
{
    self = [super init];
    if (nil == self)
    {
        return nil;
    }
    
    self->windowCount = 0;
    return self;
}

- (void)windowCreated:(NSWindow*)window
{
    assert(window);
    
    [window setDelegate:self];
    
    assert(self->windowCount < ~0u);
    self->windowCount += 1;
}

- (void)windowWillClose:(NSNotification*)notification
{
    
}

- (BOOL)windowShouldClose:(NSWindow*)window
{
    assert(window);
    
    [window setDelegate:nil];
    
    assert(self->windowCount);
    self->windowCount -= 1;
    
    if (self->windowCount == 0)
    {
        [NSApp terminate:self];
        return false;
    }
    
    return true;
}

- (void)windowDidResize:(NSNotification*)notification
{
    
}

- (void)windowDidBecomeKey:(NSNotification*)notification
{
    
}

- (void)windowDidResignKey:(NSNotification*)notification
{
    
}

@end

#endif /* PLATFORM_OSX */
