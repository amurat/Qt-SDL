#import <Cocoa/Cocoa.h>

void* GetNativeWindowHandleFromNSView(void *view)
{
    NSView* v = (NSView*)view;
    [v setWantsLayer:TRUE];
    return (__bridge void *)[v layer];
}

void* GetNativeWindowHandleFromNSWindow(void *window)
{
    NSWindow* nsw = (NSWindow*)window;
    return GetNativeWindowHandleFromNSView(nsw.contentView);
}
