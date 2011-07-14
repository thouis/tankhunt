/* data.c - declarations for globals */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

int text_height = 0;

Display *display;
Window  mainwin;
GC gc, erase_gc;
Pixmap pixmaps[256];
