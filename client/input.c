/* input.c - handle input from display and server */

#include <sys/time.h>
#include <math.h>
#include <X11/Xlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "data.h"
#include "defs.h"
#include "packets.h"

static int socket_fd;
char keymap[128];

static void
    setup_keymap()
{
    char *key_env, *c;
    int i;

    for (i = 0; i < 128; i++)
	keymap[i] = (char) i;

    key_env = getenv(KEY_MAP_VARIABLE);
    if (key_env == NULL)
	return;
    
    for (c = key_env; (*c != '\0') && (*(c + 1) != '\0'); c += 2)
	keymap[*c] = *(c + 1);
}

/* send a packet to the server */
static void
    send_packet(packet)
struct cpacket *packet;
{
    struct cpacket mypacket;

    bcopy (packet, &mypacket, sizeof (struct cpacket));
    mypacket.type = htonl(mypacket.type);
    write(socket_fd, (char *) &mypacket, sizeof(struct cpacket));
}

/* 
 * given x and y coordinates of a mouse press within the main window,
 * this function determines the direction (0-255) of that press
 * relative to the center of the screen
 */
u_char
    determine_dir(x,y)
int x;
int y;
{
    register double angle;
    int ret;
    
    x -= HALF_WIDTH;
    y -= HALF_HEIGHT;

    /* invert y */
    y = -y;

    angle = atan2((double) y, (double) x);
    return ((u_char) ((angle * 256.0) / (M_PI * 2.0)));
}

/* do an action for a certain keypress */
static void
    handle_keypress(key, x, y)
char key;
int x, y;
{
    static char last_speed = 0;
    struct cpacket packet;
    static char *negative_speeds = ")!@#$%^&*(";

    key = keymap[key];
    switch (key)
    {
    case 'Q':
	packet.type = C_EXIT;
	send_packet(&packet);
	exit (0);
	break;
    case '>':
	packet.type = C_SPEED;
	last_speed++;
	packet.data = last_speed;
	break;
    case '<':
	packet.type = C_SPEED;
	last_speed--;
	packet.data = last_speed;
	break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
	packet.type = C_SPEED;
	packet.data = last_speed = key - '0';
	break;
    case '!':
    case '@':
    case '#':
    case '$':
    case '%':
    case '^':
    case '&':
    case '*':
    case '(':
    case ')':
	packet.type = C_SPEED;
	packet.data = last_speed = 
	    (char) (negative_speeds - strchr(negative_speeds, key));
	break;
    case 'f':
	packet.type = C_FIRE;
	break;
    case 'b': 
	packet.type = C_TURN_FIRE;
	packet.data = determine_dir(x, y);
	break;
    case 'k': 
	packet.type = C_TURN;
	packet.data = determine_dir(x, y);
	break;
    default:
	XBell(display, 0);
	return;
    }
    
    send_packet(&packet);
}

/*
 * loop forever
 * wait for input from the server, handle input from display or server.
 */
void 
    input_loop(player_id, fd)
char *player_id;
int fd;
{
    int i;
    XEvent event;
    struct cpacket packet;
    struct cpacket ack_packet;
    struct timeval tval;
    fd_set rfds;
    char buf[MAX_PACKET_SIZE];
    KeySym key;
    char   text;

    socket_fd = fd;

    /* set up the keymap */
    setup_keymap();

    printf("Calling server...\n");

    /* this acts as the login packet, too. */
    ack_packet.type = C_ACK;
    for (i = 0; i < NAME_LENGTH; i++)
	ack_packet.name[i] = ' ';
    bcopy(player_id, ack_packet.name, 
	  (NAME_LENGTH > strlen(player_id)) ? strlen(player_id) : NAME_LENGTH);

    /* empty the event queue */
    while (XPending(display)) 
	XNextEvent(display, &event); 

    /* only wait a second for input from the server, then send another ack */
    tval.tv_sec= 1;
    tval.tv_usec = 0;

    while (1) 
    {
	/*
	 * send to the server.  this will both connect us and keep us
	 * from being ghostbusted.  
	 */
	send_packet(&ack_packet);

	FD_ZERO(&rfds);             /* set all bits to off */
	FD_SET(fd, &rfds);          /* only watch the server */

	select(255, &rfds, (fd_set *) 0, (fd_set *) 0, &tval);

	/* 
	 * we tried to do a select on the display's fd, but for some
	 * reason events  were being buffered.  XPending finds them
	 * immediately, so this works.  there is probably an X call
	 * that could fix the problem.
	 */
	while (XPending(display)) 
	{ 

	    XNextEvent(display, &event); 
	    switch (event.type) 
	    {
	    case KeyPress:
		if (XLookupString(&event, &text, 1, &key, 0))
		    handle_keypress(text, event.xkey.x, event.xkey.y);
		break;
	    case ButtonPress: 
		switch (event.xbutton.button) 
		{
		case 1: /* fire */
		    packet.type = C_FIRE;
		    send_packet(&packet);
		    break;
		case 2: /* turn while firing */
		    packet.type = C_TURN_FIRE;
		    packet.data = determine_dir(event.xbutton.x, 
						event.xbutton.y);
		    send_packet(&packet);
		    break;
		case 3: /* turn */
		    packet.type = C_TURN;
		    packet.data = determine_dir(event.xbutton.x, 
						event.xbutton.y);
		    send_packet(&packet);
		    break;
		}
		break;
	    }
	}

	if (FD_ISSET(fd, &rfds))
	{
	    if (read(fd, buf, MAX_PACKET_SIZE) < 0)  
	    {
		perror("Read");
		exit(1);
	    }
	    
	    /* re-draw everything */
	    update_display(buf);
	}

    }
}
