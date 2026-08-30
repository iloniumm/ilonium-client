#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Window find_window(Display *display, Window local_root, const char *name) {
    Window root, parent;
    Window *children = NULL;
    unsigned int nchildren = 0;
    if (XQueryTree(display, local_root, &root, &parent, &children, &nchildren) == 0) {
        return None;
    }
    for (unsigned int i = 0; i < nchildren; i++) {
        char *window_name = NULL;
        if (XFetchName(display, children[i], &window_name) != 0 && window_name != NULL) {
            if (strstr(window_name, name) != NULL) {
                Window win = children[i];
                printf("Matched by name: %s (ID: %lu)\n", window_name, win);
                XFree(window_name);
                XFree(children);
                return win;
            }
            XFree(window_name);
        }
        Window child = find_window(display, children[i], name);
        if (child != None) {
            XFree(children);
            return child;
        }
    }
    if (children) XFree(children);
    return None;
}

void send_event_recursive(Display *display, Window win, XKeyEvent *event, long mask) {
    event->window = win;
    XSendEvent(display, win, True, mask, (XEvent *)event);
    
    Window root, parent;
    Window *children = NULL;
    unsigned int nchildren = 0;
    if (XQueryTree(display, win, &root, &parent, &children, &nchildren) != 0 && children != NULL) {
        for (unsigned int i = 0; i < nchildren; i++) {
            send_event_recursive(display, children[i], event, mask);
        }
        XFree(children);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <keysym_name>\n", argv[0]);
        return 1;
    }

    Display *display = XOpenDisplay(NULL);
    if (!display) {
        printf("Cannot open display\n");
        return 1;
    }
    
    Window window = find_window(display, DefaultRootWindow(display), "Retrocycles");
    if (window == None) {
        window = find_window(display, DefaultRootWindow(display), "Armagetron");
    }
    if (window == None) {
        int revert_to;
        XGetInputFocus(display, &window, &revert_to);
        printf("Fallback to input focus window (ID: %lu)\n", window);
    }
    
    if (window == None) {
        printf("No window found\n");
        XCloseDisplay(display);
        return 1;
    }
    
    KeySym keysym = XStringToKeysym(argv[1]);
    if (keysym == NoSymbol) {
        printf("Unknown keysym: %s\n", argv[1]);
        XCloseDisplay(display);
        return 1;
    }

    KeyCode keycode = XKeysymToKeycode(display, keysym);
    if (keycode == 0) {
        printf("No keycode for keysym: %s\n", argv[1]);
        XCloseDisplay(display);
        return 1;
    }

    // Raise window and set focus to be safe
    XRaiseWindow(display, window);
    XSetInputFocus(display, window, RevertToParent, CurrentTime);
    XFlush(display);
    usleep(100000); // 100ms for window manager to react

    XKeyEvent event;
    event.display = display;
    event.window = window;
    event.root = DefaultRootWindow(display);
    event.subwindow = None;
    event.time = CurrentTime;
    event.x = 1;
    event.y = 1;
    event.x_root = 1;
    event.y_root = 1;
    event.state = 0;
    event.keycode = keycode;
    event.same_screen = True;

    // Press
    event.type = KeyPress;
    send_event_recursive(display, window, &event, KeyPressMask);
    XFlush(display);
    
    usleep(50000); // 50ms
    
    // Release
    event.type = KeyRelease;
    send_event_recursive(display, window, &event, KeyReleaseMask);
    XFlush(display);
    
    XCloseDisplay(display);
    return 0;
}
