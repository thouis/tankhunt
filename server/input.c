/* input.c - input loop and related functions */

#include <stdio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>

#include "defs.h"
#include "data.h"
#include "struct.h"
#include "packets.h"
#include "sintab.h"

/*
 * place a player in the arena.  check to make sure they don't appear 
 * on anyone else
 */
void
    place_player(p)
struct player *p;
{
    int i;
    struct player *other;
    int done = 0;

    while (! done)
    {
	/* choose a random place */
	p->x = random() % arena_size;
	p->y = random() % arena_size;

	done = 1;
	for (i = 0, other = players; done && (i < num_players); i++, other++)
	{
	    if (other == p)
		continue;

	    /* someone else here? */
	    if (((p->x / BOX_SIZE) == (other->x / BOX_SIZE)) &&
		((p->y / BOX_SIZE) == (other->y / BOX_SIZE)))
		done = 0;
	}
    }

    /* found a place, assign our other values */
    p->x = p->x - (p->x % BOX_SIZE) + (BOX_SIZE >> 1);
    p->y = p->y - (p->y % BOX_SIZE) + (BOX_SIZE >> 1);
    p->status = STAT_ALIVE;
    p->des_speed = p->speed = 0;
    p->des_dir = p->dir = 64;
}

/* 
 * fire a bullet from player i 
 */
static void 
    fire(i)
int i;
{
    struct player *p;
    struct bullet *b;

    p = &(players[i]);

    /* check that we don't already have a bullet and that we're alive */
    /* to make it possible to shoot more, this should check a count */
    if ((p->has_bullet) || (p->status != STAT_ALIVE))
	return;

    /* increment global bullet count */
    num_bullets++;

    b = &(bullets[i]);

    /* make sure to put it outside the tank, else we blow up */
    b->x = p->x + COS(p->dir, TANK_RADIUS);
    b->y = p->y + SIN(p->dir, TANK_RADIUS);

    if (b->x < 0)
	b->x += arena_size;
    if (b->y < 0)
	b->y += arena_size;
    if (b->x > arena_size)
	b->x -= arena_size;
    if (b->y > arena_size)
	b->y -= arena_size;

    /* b->box is used later on on wall crossing checks */
    b->box_x = b->x / BOX_SIZE;
    b->box_y = b->y / BOX_SIZE;
    b->dir = p->dir;
    b->status = STAT_ALIVE;

    /* to make it possible to shoot more, this should have a count */
    p->has_bullet = 1;
}

/*
 * sit idle, waiting for input.  the update routine is called via a
 * signal, so we will get there eventually. 
 * we can use recvfrom, since it will restart if interrupted by a
 * signal.  
 */
void 
    input_loop()
{
    struct cpacket cp;
    struct sockaddr_in from_addr;
    int from_len;
    int who;
    struct player *p;
    fd_set rfds;
    int num_bytes;
    int speed;

    /*
     * basically, we sit here waiting for a packet to arrive... when
     * it does, we check who it's from.  If they're not in the player
     * list, we add them, and place them in the arena.  We also update
     * last_ack in this function.
     */
       
    /* do this forever */
    while (1)
    {
	from_len = sizeof (struct sockaddr_in);
	num_bytes = recvfrom(port_fd, &cp, sizeof (struct cpacket), 
			     0, (struct sockaddr *) &from_addr,
			     &from_len);
	if (num_bytes < 0)
	{
	    if (errno == EINTR) continue; /* signal going off */

	    perror("recvfrom");
	    exit (1);
	}

	if (num_bytes != sizeof (struct cpacket))
	{
	    fprintf(stderr, "Weird packet!\n");
	    continue;
	}

	/*
	 * try to find this player, via address, in the list of
	 * current players 
	 */
	for (who = 0, p = players; who < num_players; who++, p++)
	{
	    if ((from_addr.sin_port == p->client_addr.sin_port) &&
		(from_addr.sin_addr.s_addr == p->client_addr.sin_addr.s_addr))
		break;
	}

	/* didn't find him, must be new */
	if (who == num_players)
	{
	    fprintf(stderr, "Adding player %d %.3s\n", who,
		    cp.name);

	    num_players++;
	    place_player(p);
	    p->has_bullet = 0;
	    bcopy((char *) &from_addr, (char *) &(p->client_addr),
		  sizeof (struct sockaddr_in));
	    /* get their name */
	    bcopy(cp.name, p->name, NAME_LENGTH);
	}

	/* stop the ghostbuster */
	p->last_ack = 0;

	/* handle packet */
	switch (ntohl(cp.type))
	{
	case C_ACK:
	    break;
	case C_FIRE:
	    fire(who);
	    break;
	case C_TURN_FIRE:
	    fire(who);
	    p->des_dir = cp.data;
	    break;
	case C_TURN:
	    p->des_dir = cp.data;
	    break;
	case C_SPEED:
	    /* Assign to an int, some machines are stupid about this */
	    speed = (int) cp.data;
	    if (speed > MAX_SPEED) speed = MAX_SPEED;
	    if (speed < -MAX_SPEED) speed = -MAX_SPEED;
	    p->des_speed = SPEED_SCALE * speed;
	    break;
	case C_EXIT:
	    p->last_ack = TIMEOUT;
	    break;
	default:
	    fprintf(stderr, "Unkown packet type!\n");
	    break;
	}
    }
}
