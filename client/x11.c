/* x11.x - connect to and initialize server */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "defs.h"
#include "data.h"
#include "packets.h"
#include "tank.h"

void rotate_bitmap();

/* build the array of rotated bitmaps */
void
    build_rotations()
{
    int i;
    u_char *bits;

    printf("Creating bitmaps...\n");

    bits = (u_char *) malloc((tank_width * tank_height) / 8 + 1);
    for (i = 0 ; i < 256; i++)
    {
	rotate_bitmap((u_char) i, tank_bits, bits, tank_width);
	pixmaps[i] = 
	    XCreateBitmapFromData(display, 
				  RootWindow(display, DefaultScreen(display)),
				  bits, tank_width, tank_height);
    }
}

/* 
 * connect to the display, create the window, build the drawing and
 * clearing GCs, build the rotations of the tankm, and map the window
 */
void
    init_display(display_name) 
char *display_name;
{
    int screen;
    XEvent garbage;
    XGCValues gcvalues;
    XWMHints wmhints;
    XFontStruct *xfs;
        
    display = XOpenDisplay(display_name);
    if (display == (Display *) NULL) 
    {
	fprintf(stderr, "Could not connect to display %s\n",
		XDisplayName(display_name));
	exit (1);
    }

    screen = DefaultScreen(display);
    mainwin = XCreateSimpleWindow(display, RootWindow(display, screen),
				  0, 0, WIN_WIDTH, WIN_HEIGHT, 0, 
				  WhitePixel(display, screen), 
				  BlackPixel(display,screen)); 

    wmhints.flags = InputHint;
    wmhints.input = True;
    XSetWMHints(display, mainwin, &wmhints);

    /* create the drawing GC */
    gcvalues.foreground = WhitePixel(display, screen);
    gcvalues.background = BlackPixel(display, screen);
    gcvalues.font = XLoadFont(display, "fixed");
    xfs = XQueryFont(display, gcvalues.font);

    /* we need this information to clear the font later on */
    text_height = xfs->ascent + xfs->descent;

    gc = XCreateGC(display, RootWindow(display, screen),
		   GCForeground | GCBackground | GCFont, &gcvalues);

    /* create the erasing GC */
    gcvalues.foreground = BlackPixel(display, screen);
    gcvalues.background = WhitePixel(display, screen);
    erase_gc = XCreateGC(display, RootWindow(display, screen),
		   GCForeground | GCBackground, &gcvalues); 

    /* rotate the bitmap */
    build_rotations();

    /* make sure the window actually is on the screen before going on */
    XSelectInput(display, mainwin, ExposureMask);
    XMapWindow(display, mainwin);
    XNextEvent(display, &garbage);

    /* clear the input queue */
    XSync(display, 1);

    /* begin accepting player input */
    XSelectInput(display, mainwin, ButtonPressMask | KeyPressMask);
}

