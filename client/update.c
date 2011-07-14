/* update.c - draw walls, tanks, bullets and explosions */

#include <stdio.h>
#include <X11/Xlib.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "defs.h"
#include "data.h"
#include "packets.h"
#include "tank.h"

/*
 * each function in here uses the same strategy.  it first undraws all
 * the old stuff, using static data it defined previously, then
 * rebuilds the data with the current information, then draws that
 * data and returns.
 */

/*
 * draw walls
 */
static void
    draw_walls(pheader) 
struct packet_header *pheader;
{
    int i;
    int xbase, ybase;
    int xcount, ycount;
    XSegment *s;
    u_char *wall;
    static int num_segs = 0;
    static XSegment segs[WALL_NUM * 2];

    if (num_segs)
	XDrawSegments(display, mainwin, erase_gc, segs, num_segs);

    num_segs = 0;
    s = segs;

    /*
     * determine how much we need to shift the walls so that they have
     * the correct position on the screen 
     */
    xbase = -BOX_SIZE - (ntohs(pheader->rel_x) - BOX_SIZE / 2);
    ybase = -BOX_SIZE - (BOX_SIZE / 2 - ntohs(pheader->rel_y));

    /* 
     * go through the wall data and draw the walls, w/clipping
     * (to prevent someone from just resizing the window and getting
     * more information)
     */
    xcount = 0;
    ycount = (VIEW_SIZE + 1) * BOX_SIZE;
    for (i = 0, wall = pheader->walls; i < WALL_NUM; i++, wall++) 
    {
	int xfrom, xto, yfrom, yto;

	/* 
	 * now figure out the actual starting and ending points of
	 * the lines we need to draw.  check them against the borders
	 * of the window and cut off anything going outside of it 
	 */
	if (*wall & BOTTOM)
	{
	    xfrom = xbase + xcount;
	    xto   = xfrom + BOX_SIZE;
	    yfrom = ybase + ycount + BOX_SIZE;
	    if (xfrom < 0) xfrom = 0; 
	    if (xfrom > WIN_WIDTH) xfrom = WIN_WIDTH;
	    if (xto < 0) xto = 0;
	    if (xto > WIN_WIDTH) xto = WIN_WIDTH;
	    if ((xfrom != xto) && (yfrom >= 0) && (yfrom < WIN_HEIGHT)) 
	    {
		s->x1 = xfrom;
		s->x2 = xto;
		s->y1 = s->y2 = yfrom;
		num_segs++;
		s++;
	    }
	} 
	if (*wall & RIGHT)
	{
	    yfrom = ybase + ycount;
	    yto   = yfrom + BOX_SIZE;
	    xfrom = xbase + xcount + BOX_SIZE;
	    if (yfrom < 0) yfrom = 0; 
	    if (yfrom > WIN_HEIGHT) yfrom = WIN_HEIGHT;
	    if (yto < 0) yto = 0;
	    if (yto > WIN_HEIGHT) yto = WIN_HEIGHT;
	    if ((yfrom != yto) && (xfrom >= 0) && (xfrom < WIN_WIDTH)) 
	    {
		s->x1 = s->x2 = xfrom;
		s->y1 = yfrom;
		s->y2 = yto;
		num_segs++;
		s++;
	    }
	}
	ycount -= BOX_SIZE;
	if (ycount < 0) 
	{
	    ycount = (VIEW_SIZE + 1) * BOX_SIZE;
	    xcount += BOX_SIZE;
	}
    }
    if (num_segs)
	XDrawSegments(display, mainwin, gc, segs, num_segs);
} 
	    
		
struct player {
    int x, y;
    u_char dir;
    u_char pad[3];
};

/*
 * draw tanks
 */
static void
    update_tanks(pheader, ptank) 
struct packet_header *pheader;
struct packet_tank   *ptank;
{
    int i;
    struct player *p;
    static struct player players[MAX_PLAYERS];
    static int num_tanks = 0;
    
    for (i = 0, p = players; i < num_tanks; i++, p++)
	XClearArea(display, mainwin, p->x, p->y,
		   tank_width, tank_height + text_height + 3, 0);

    num_tanks = ntohl(pheader->num_tanks);
    for (i = 0, p = players; i < num_tanks; i++, ptank++, p++) 
    {
	p->x = HALF_WIDTH + ntohs(ptank->rel_x) - (tank_width >> 1);
	p->y = HALF_HEIGHT - ntohs(ptank->rel_y) - (tank_height >> 1);
	p->dir = ptank->dir;

	XCopyPlane(display, pixmaps[p->dir], mainwin, gc, 
		   0, 0, tank_width, tank_height, p->x, p->y, 1);
	XDrawString(display, mainwin, gc, 
		    p->x + 1, p->y + tank_height + text_height,
		    ptank->name, NAME_LENGTH);
    }
}


#define BULLET_OFFSET 2
#define BULLET_DIAMETER 5
/*
 * draw bullets
 */
static void
    update_bullets(pheader, pbullet)
struct packet_header *pheader;
struct packet_bullet *pbullet;
{
    int i;
    XArc *b;
    static XArc bullets[MAX_BULLETS]; 
    static int num_bullets = 0;

    if (num_bullets)
	XFillArcs(display, mainwin, erase_gc, bullets, num_bullets);

    num_bullets = ntohl(pheader->num_bullets);
    for (i = 0, b = bullets; i < num_bullets; i++, b++, pbullet++) 
    {
	b->x = HALF_WIDTH + ntohs(pbullet->rel_x) - BULLET_OFFSET;
	b->y = HALF_WIDTH - ntohs(pbullet->rel_y) - BULLET_OFFSET;
	b->width = b->height = BULLET_DIAMETER;
	b->angle1 = 0; 
	b->angle2 = 360 * 64;

	pbullet++;
    }

    if (num_bullets)
	XFillArcs(display, mainwin, gc, bullets, num_bullets);
}

/*
 * draw explosions
 */
static void
    update_explosions(pheader, pexplode)
struct packet_header  *pheader;
struct packet_explosion *pexplode;
{
    int i;
    XArc *e;
    static XArc explosions[MAX_BULLETS];
    static int num_explosions = 0;
    
    if (num_explosions)
	XFillArcs(display, mainwin, erase_gc, explosions, num_explosions);

    num_explosions = ntohl(pheader->num_explosions);
    for (i = 0, e = explosions; i < num_explosions; i++, e++, pexplode++)
    {
	e->x = HALF_WIDTH + ntohs(pexplode->rel_x) - pexplode->radius; 
	e->y = HALF_HEIGHT - ntohs(pexplode->rel_y) - pexplode->radius;
	e->width = e->height = (pexplode->radius << 1);
	e->angle1 = 0; 
	e->angle2 = 360 * 64;
    }

    if (num_explosions)
	XFillArcs(display, mainwin, gc, explosions, num_explosions);
}
	

/* 
 * find the offsets to the data, call the drawing functions
 */
void
    update_display(buf)
char *buf;
{
    int i;
    struct packet_header    *pheader;
    struct packet_tank      *ptank;
    struct packet_bullet    *pbullet;
    struct packet_explosion *pexplode;

    pheader = (struct packet_header *) buf; 
    i = sizeof(struct packet_header); 
    ptank  = (struct packet_tank *) (buf + i);
    i += sizeof(struct packet_tank) * ntohl(pheader->num_tanks);
    pbullet = (struct packet_bullet *) (buf + i);
    i += sizeof(struct packet_bullet) * ntohl(pheader->num_bullets);
    pexplode = (struct packet_explosion *) (buf + i);

    draw_walls(pheader);
    update_bullets(pheader, pbullet);
    update_explosions(pheader, pexplode);
    update_tanks(pheader, ptank);

    XSync(display, 0); 
}
