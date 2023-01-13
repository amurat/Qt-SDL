// standalone example application for OSX + OpenGL2
#import <Cocoa/Cocoa.h>

#include "rendergl.h"
#include "glescontext.h"
#include "glesdebug.h"

//-----------------------------------------------------------------------------------
// AppView
//-----------------------------------------------------------------------------------

@interface AppView : NSView
{
    NSTimer*    animationTimer;
    GLESContext* context_;
    RenderGL* rendergl_;
}
@end

@implementation AppView

extern void* GetNativeWindowHandleFromNSView(void *view);
extern void* GetNativeWindowHandleFromNSWindow(void *window);

-(void)initialize
{
    if (context_) {
        return;
    }
    setenv("GALOGEN_GL4ES_LIBRARY", "libGL4ES.dylib", 1);

    void* windowView = self;
    void* windowHandle = GetNativeWindowHandleFromNSView(windowView);
    
    context_ = new GLESContext(windowHandle);
    context_->create();
    
    EnableGLESDebugHandler();

    rendergl_ = new RenderGLES2();
    rendergl_->setup();
}

-(void)updateAndDrawDemoView
{
    // opengl render
    NSRect r;
    NSScreen* screen = [NSScreen mainScreen];
    r = [screen convertRectToBacking :self.bounds];
    rendergl_->render(r.size.width, r.size.height);
    context_->swapBuffers();

    if (!animationTimer)
        animationTimer = [NSTimer scheduledTimerWithTimeInterval:0.017 target:self selector:@selector(animationTimerFired:) userInfo:nil repeats:YES];
}

-(void)reshape                              { [super reshape];
    //[[self openGLContext] update];
    [self updateAndDrawDemoView];
}

-(void)drawRect:(NSRect)bounds              { [self updateAndDrawDemoView]; }
-(void)animationTimerFired:(NSTimer*)timer  { [self setNeedsDisplay:YES]; }
-(void)dealloc                              { animationTimer = nil; }

@end

//-----------------------------------------------------------------------------------
// AppDelegate
//-----------------------------------------------------------------------------------

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, readonly) NSWindow* window;
@end

@implementation AppDelegate
@synthesize window = _window;

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
    return YES;
}

-(NSWindow*)window
{
    if (_window != nil)
        return (_window);

    NSRect viewRect = NSMakeRect(100.0, 100.0, 100.0 + 1280.0, 100 + 720.0);

    _window = [[NSWindow alloc] initWithContentRect:viewRect styleMask:NSWindowStyleMaskTitled|NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskResizable|NSWindowStyleMaskClosable backing:NSBackingStoreBuffered defer:YES];
    [_window setTitle:@"OSX+OpenGL Example"];
    [_window setAcceptsMouseMovedEvents:YES];
    [_window setOpaque:YES];
    [_window makeKeyAndOrderFront:NSApp];

    return (_window);
}

-(void)setupMenu
{
    NSMenu* mainMenuBar = [[NSMenu alloc] init];
    NSMenu* appMenu;
    NSMenuItem* menuItem;

    appMenu = [[NSMenu alloc] initWithTitle:@"OSX+OpenGL2 Example"];
    menuItem = [appMenu addItemWithTitle:@"OSX+OpenGL2 Example" action:@selector(terminate:) keyEquivalent:@"q"];
    [menuItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand];

    menuItem = [[NSMenuItem alloc] init];
    [menuItem setSubmenu:appMenu];

    [mainMenuBar addItem:menuItem];

    appMenu = nil;
    [NSApp setMainMenu:mainMenuBar];
}

-(void)dealloc
{
    _window = nil;
}

-(void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Make the application a foreground application (else it won't receive keyboard events)
    ProcessSerialNumber psn = {0, kCurrentProcess};
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);

    // Menu
    [self setupMenu];

    AppView* view = [[AppView alloc] initWithFrame:self.window.frame];
    [self.window setContentView:view];
    [view initialize];
}

@end

//-----------------------------------------------------------------------------------
// Application main() function
//-----------------------------------------------------------------------------------

int main(int argc, const char* argv[])
{
    @autoreleasepool
    {
        NSApp = [NSApplication sharedApplication];
        AppDelegate* delegate = [[AppDelegate alloc] init];
        [[NSApplication sharedApplication] setDelegate:delegate];
        [NSApp run];
    }
    return NSApplicationMain(argc, argv);
}
