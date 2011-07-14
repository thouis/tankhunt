/* update.c - update world state and send to clients */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "defs.h"
#include "struct.h"
#include "data.h"
#include "sintab.h"
#include "packets.h"
#include "fast_dis.h"

static void update_clients();
u_char fetch_wall_data();
u_char get_one_wall();
u_char fast_atan();
void place_player();

/* return whether or not the bullet just hit a wall. */
/* this is probably the most heinous function in this entire game */
/*
 * i'll attempt a reasonable explanation.
 * basically, it checks if we've crossed the boundary of a square.  if
 * we have, it checks which of the boundaries we crossed.  
 *
 * if we didn't move diagonally, this is easy.  the only wall we need
 * to check is the one we crossed over when we left.
 *
 * if we've moved diagonally, it gets ugly.  we have to figure out
 * which of the two walls we crossed.  to do this, we check the angle
 * of the corner we passed nearest to as compared to the old position
 * against the angle the bullet is travelling at.  this gives enough
 * information to know which two walls to check.  
 *
 * it should probably be rewritten to just generate two masks, say
 * old_square_mask and new_square_mask, and then just check and return
 * at the end, rather than having returns all through it, but i'm not
 * sure (a) it would be as fast (probably wouldn't), and (b) i don't
 * know if it would be particularly easier to understand.    
 *
 * to make it possible to move the bullet onto the wall that it hit
 * (rather than  having the explosion center off the wall), we return
 * values for which walls were hit in wall_prev and wall_cur.
 * wall_prev is the wall that was passed through when leaving the
 * previous square, wall_cur the wall we passed through entering.
 * the only time wall_cur needs to eb set is in the cases where we
 * passed near a corner.
 */
static u_char
    check_bullet(b, wall_prev, wall_cur)
struct bullet *b;
u_char *wall_prev, *wall_cur;
{
    int new_x, new_y;
    int rel_x, rel_y;

    new_x = b->x / BOX_SIZE;
    new_y = b->y / BOX_SIZE;
    if(b->x < 0) new_x--;
    if(b->y < 0) new_y--;

    if ((new_x == b->box_x) && (new_y == b->box_y))
	return (0);

    if (new_x < b->box_x)
    {
	if (new_y < b->box_y)
	{
	    /* bottom left */
	    rel_x = b->x % BOX_SIZE;
	    rel_y = b->y % BOX_SIZE;

	    if (fast_atan(-rel_x, -rel_y) > b->dir)
	    {
		*wall_prev = get_one_wall(b->box_x, b->box_y) & LEFT;
		*wall_cur = get_one_wall(new_x, new_y) & TOP;
		return (*wall_prev || *wall_cur);
	    }

	    *wall_prev = get_one_wall(b->box_x, b->box_y) & BOTTOM;
	    *wall_cur = get_one_wall(new_x, new_y) & RIGHT;
	    return (*wall_prev || *wall_cur);
	}
	if (new_y > b->box_y)
	{
	    /* top left */
	    rel_x = b->x % BOX_SIZE;
	    rel_y = b->y % BOX_SIZE;

	    if (fast_atan(-rel_x, BOX_SIZE - rel_y) > b->dir)
	    {
		*wall_prev = get_one_wall(b->box_x, b->box_y) & TOP;
		*wall_cur = get_one_wall(new_x, new_y) & RIGHT;
		return (*wall_prev || *wall_cur);
	    }

	    *wall_prev = get_one_wall(b->box_x, b->box_y) & LEFT;
	    *wall_cur = get_one_wall(new_x, new_y) & BOTTOM;
	    return (*wall_prev || *wall_cur);
	}

	*wall_prev = get_one_wall(b->box_x, b->box_y) & LEFT;
	return (*wall_prev);
    }
    if (new_x > b->box_x)
    {
	if (new_y < b->box_y)
	{
	    /* bottom right */
	    rel_x = b->x % BOX_SIZE;
	    rel_y = b->y % BOX_SIZE;

	    if (fast_atan(BOX_SIZE - rel_x, -rel_y) > b->dir)
	    {
		*wall_prev = get_one_wall(b->box_x, b->box_y) & BOTTOM;
		*wall_cur = get_one_wall(new_x, new_y) & LEFT;
		return (*wall_prev || *wall_cur);
	    }

	    *wall_prev = get_one_wall(b->box_x, b->box_y) & RIGHT;
	    *wall_cur = get_one_wall(new_x, new_y) & TOP;
	    return (*wall_prev || *wall_cur);
	}
	if (new_y > b->box_y)
	{
	    /* top right */

	    rel_x = b->x % BOX_SIZE;
	    rel_y = b->y % BOX_SIZE;

	    if (fast_atan(BOX_SIZE - rel_x, BOX_SIZE - rel_y) > b->dir)
	    {
		*wall_prev = get_one_wall(b->box_x, b->box_y) & RIGHT;
		*wall_cur = get_one_wall(new_x, new_y) & BOTTOM;
		return (*wall_prev || *wall_cur);
	    }		

	    *wall_prev = get_one_wall(b->box_x, b->box_y) & TOP;
	    *wall_cur = get_one_wall(new_x, new_y) & LEFT;
	    return (*wall_prev || *wall_cur);
	}

	*wall_prev = get_one_wall(b->box_x, b->box_y) & RIGHT;
	return (*wall_prev);
    }
    if (new_y < b->box_y)
    {
	*wall_prev = get_one_wall(b->box_x, b->box_y) & BOTTOM;
	return (*wall_prev);
    }

    *wall_prev = get_one_wall(b->box_x, b->box_y) & TOP;
    return (*wall_prev);
}

/*
 * update the position/rotation of tanks, move bullets, and animate explosions.
 * calls update_clients() at the end.
 */
void 
    update()
{
    struct player *p;
    struct bullet *b;
    int i;
    int new_x, new_y;
    u_char old_walls, new_walls;
    int del_x, del_y;

    sequence_number++;

    for (i = 0, p = players, b = bullets; i < num_players; i++, p++, b++)
    {
	/* is connection dead? */
	if (p->last_ack++ == TIMEOUT)
	{
	    fprintf(stderr, "Removing player %d %.3s\n", i, players[i].name);
	    num_players--;
	    if (p->has_bullet)
		num_bullets--;
	    bcopy(&(players[num_players]), p,
		  sizeof (struct player));
	    bcopy(&(bullets[num_players]), b,
		  sizeof (struct bullet));
	    i--;
	    p--;
	    b--;
	    continue;
	}
	
	/* 
	 * handle bullets 
	 * do this before tanks so that bullets continue to move
	 * while their owner explodes.
	 */
	if (p->has_bullet)
	{
	    if (b->status == STAT_ALIVE)
	    {
		b->x += COS(b->dir, BULLET_SPEED);
		b->y += SIN(b->dir, BULLET_SPEED);

		if (check_bullet(b, &old_walls, &new_walls))
		{
		
		    b->status = STAT_EXPLODE;
		    b->explosion_radius = 0;
		    
		    /* make the mod math work right */
		    b->x += BOX_SIZE;
		    b->y += BOX_SIZE;

		    /* move the bullet to the wall */
		    switch (old_walls)
		    {
		    case TOP:
			del_y = -(b->y % BOX_SIZE);
			del_x = (del_y * COS(b->dir, BULLET_SPEED)) / 
				 SIN(b->dir, BULLET_SPEED);
			break;
		    case BOTTOM:
			del_y = BOX_SIZE - (b->y % BOX_SIZE);
			del_x = (del_y * COS(b->dir, BULLET_SPEED)) / 
				 SIN(b->dir, BULLET_SPEED);
			break;
		    case LEFT:
			del_x = BOX_SIZE - (b->x % BOX_SIZE);
			del_y = (del_x * SIN(b->dir, BULLET_SPEED)) / 
				 COS(b->dir, BULLET_SPEED);
			break;
		    case RIGHT:
			del_x = -(b->x % BOX_SIZE);
			del_y = (del_x * SIN(b->dir, BULLET_SPEED)) / 
				 COS(b->dir, BULLET_SPEED);
			break;
		    default: /* none of the previous walls are set */
			switch (new_walls)
			{
			case BOTTOM:
			    del_y = -(b->y % BOX_SIZE);
			    del_x = (del_y * COS(b->dir, BULLET_SPEED)) / 
				     SIN(b->dir, BULLET_SPEED);
			    break;
			case TOP:
			    del_y = BOX_SIZE - (b->y % BOX_SIZE);
			    del_x = (del_y * COS(b->dir, BULLET_SPEED)) / 
				     SIN(b->dir, BULLET_SPEED);
			    break;
			case RIGHT:
			    del_x = BOX_SIZE - (b->x % BOX_SIZE);
			    del_y = (del_x * SIN(b->dir, BULLET_SPEED)) / 
				     COS(b->dir, BULLET_SPEED);
			    break;
			case LEFT:
			    del_x = -(b->x % BOX_SIZE);
			    del_y = (del_x * SIN(b->dir, BULLET_SPEED)) / 
				     COS(b->dir, BULLET_SPEED);
			    break;
			}
		    }
		    b->x += (del_x - BOX_SIZE);
		    b->y += (del_y - BOX_SIZE);
		}

		if (b->x < 0)
		    b->x += arena_size;
		if (b->y < 0)
		    b->y += arena_size;
		if (b->x > arena_size)
		    b->x -= arena_size;
		if (b->y > arena_size)
		    b->y -= arena_size;

		b->box_x = b->x / BOX_SIZE;
		b->box_y = b->y / BOX_SIZE;
	    }
	    else
	    {
		b->explosion_radius += EXPLOSION_SPEED;
		if (b->explosion_radius > MAX_BULLET_EXPLOSION)
		{
		    p->has_bullet = 0;
		    num_bullets--;
		}
	    }
	}

	/* update explosions */
	if (p->status == STAT_EXPLODE)
	{
	    p->explosion_radius += EXPLOSION_SPEED;
	    if (p->explosion_radius > MAX_EXPLOSION_RADIUS)
		place_player(p);
	    continue;
	}
	
	/* turn tank */
	if (p->des_dir != p->dir)
	{
	    int which, diff;
	    
	    which = (p->dir > p->des_dir);
	    diff = which ? (p->dir - p->des_dir) : (p->des_dir - p->dir);

	    if ((diff < TURN_RATE) || (diff > (256 - TURN_RATE)))
		p->dir = p->des_dir;
	    else
		if ((which && (diff < 128)) ||((! which) && (diff > 128)))
		    p->dir -= TURN_RATE;
		else
		    p->dir += TURN_RATE;
	}

	/* change speed */
	if (p->speed != p->des_speed)
	    if (p->speed > p->des_speed)
		p->speed--;
	    else
		p->speed++;

	/* move tank */
	p->x += COS(p->dir, p->speed);
	p->y += SIN(p->dir, p->speed);
	
	if (p->x < 0)
	    p->x += arena_size;
	if (p->y < 0)
	    p->y += arena_size;
	if (p->x > arena_size)
	    p->x -= arena_size;
	if (p->y > arena_size)
	    p->y -= arena_size;
    }

    /* write data to clients */
    if (num_players)
	update_clients();
}

/* explode a tank, checking that he's not already exploding */
static void
    explode(p)
struct player *p;
{
    if (p->status != STAT_EXPLODE)
    {
	p->status = STAT_EXPLODE;
	p->explosion_radius = TANK_RADIUS;
    }
}

/* types to classify objects when sending to client */
#define TYPE_TANK      0
#define TYPE_BULLET    1
#define TYPE_EXPLOSION 2
/*
 * write out the updates to the clients.  also check for collsions
 * with walls, other tanks, bullets, and explosions.
 */
static void
    update_clients()
{
    int i, j, k, n;
    int diffx, diffy;
    struct player *p;
    struct bullet *b;
    struct packet_to_client *all_packets, *start, *end, *src, *dest;
    struct packet_to_client *self_locations[MAX_PLAYERS];
    struct bullet *bullet_locations[MAX_PLAYERS];
    char names[MAX_PLAYERS][NAME_LENGTH];
    struct packet_header *ph;
    char *buf;
    int can_see;
    int packet_size;

    /* the buffer for the actual building of packets */
    buf = (char *) malloc(sizeof (struct packet_header) +
			  (num_players + num_bullets) * 
			  sizeof (struct packet_to_client));

    /* 
     * we put all the tanks, bullets and explosions in this buffer,
     * and then use it to calculate visibility and relative offsets,
     * and then copy the packets we need out of this buffer and into
     * the buffer that is actually written.
     */
    all_packets = (struct packet_to_client *) 
	malloc ((num_players + num_bullets) * 
		sizeof (struct packet_to_client));

    start = all_packets;
    end = all_packets + num_players + num_bullets - 1;

    /* in all_packets, put tanks first, then bullets, then explosions */
    /* tanks and their explosions first */
    for (i = 0, p = players, n = 0; i < num_players; i++, p++)
    {
	if (p->status == STAT_ALIVE)
	{
	    /* later on, we need to know where we put this tank */
	    self_locations[i] = start;
	    start->rel_x = p->x;
	    start->rel_y = p->y;
	    start->data = p->dir;
	    start->type = TYPE_TANK;
	    bcopy(p->name, names[n], NAME_LENGTH);
	    start++;
	    n++;
	}
	else
	{
	    /* later on, we need to know where we put this tank */
	    self_locations[i] = end;
	    end->rel_x = p->x;
	    end->rel_y = p->y;
	    end->data = p->explosion_radius;
	    end->type = TYPE_EXPLOSION;
	    end--;
	}
    }

    /* then bullets and their explosions */
    for (i = 0, p = players, b = bullets; i < num_players; i++, p++, b++)
    {
	if (p->has_bullet)
	{
	    if (b->status == STAT_ALIVE)
	    {
		start->rel_x = b->x;
		start->rel_y = b->y;
		start->type = TYPE_BULLET;
		start++;
	    }
	    else
	    {
		end->rel_x = b->x;
		end->rel_y = b->y;
		end->data = b->explosion_radius;
		end->type = TYPE_EXPLOSION;
		end--;
	    }
	}
    }

    p = players;
    for (i = 0; i < num_players; i++, p++)
    {
	diffx = -(self_locations[i]->rel_x);
	diffy = -(self_locations[i]->rel_y);

	/* myself */
	ph = (struct packet_header *) buf;
	ph->seq_number = htonl(sequence_number);
	ph->rel_x = htons(p->x % BOX_SIZE);
	ph->rel_y = htons(p->y % BOX_SIZE);
	
	/* walls */
	if (fetch_wall_data(p->x / BOX_SIZE, p->y / BOX_SIZE,
			    ph->rel_x, ph->rel_y,
			    5, ph->walls))
	    explode(p);

	ph->num_tanks = 0;
	ph->num_bullets = 0;
	ph->num_explosions = 0;

	packet_size = sizeof (struct packet_header);

	dest = (struct packet_to_client *) (buf + packet_size);
	for (j = 0, src = all_packets; 
	     j < num_players + num_bullets; 
	     j++, src++)
	{
	    /* 
	     * we have to keep the rel_x, rel_y between -half_arena
	     * and half_arena 
	     */
	    src->rel_x += diffx;
	    if (src->rel_x > half_arena)
		src->rel_x -= arena_size;
	    else if (src->rel_x < -half_arena)
		src->rel_x += arena_size;
	    src->rel_y += diffy;
	    if (src->rel_y > half_arena)
		src->rel_y -= arena_size;
	    else if (src->rel_y < -half_arena)
		src->rel_y += arena_size;

	    can_see = (ABS(src->rel_x) < VIS_DISTANCE) &&
		(ABS(src->rel_y) < VIS_DISTANCE);
	    if (can_see)
	    {
		/* make sure not to blow ourselves up */
		if (self_locations[i] != src)
		    switch (src->type)
		    {
		    case TYPE_TANK:
			if (fast_dis(ABS(src->rel_x), ABS(src->rel_y)) < 
			    COLLISION_DISTANCE)
			    explode(p);
			break;
		    case TYPE_BULLET:
			if (fast_dis(ABS(src->rel_x), ABS(src->rel_y)) < 
			    TANK_RADIUS)
			    explode(p);
			break;
		    case TYPE_EXPLOSION:
			if (fast_dis(ABS(src->rel_x), ABS(src->rel_y)) < 
			    ((int) src->data + TANK_RADIUS))
			    explode(p);
			break;
		    }

		/* increment counters */
		switch (src->type)
		{
		case TYPE_TANK:
		    ph->num_tanks++;
		    break;
		case TYPE_BULLET:
		    ph->num_bullets++;
		    break;
		case TYPE_EXPLOSION:
		    ph->num_explosions++;
		    break;
		}

		dest->rel_x = htons(src->rel_x);
		dest->rel_y = htons(src->rel_y);
		dest->data = src->data;

		/* put the name in for tanks */
		if (src->type == TYPE_TANK)
		    bcopy(names[j], &(dest->type), NAME_LENGTH);

		dest++;
		packet_size += sizeof (struct packet_to_client);
	    }
	}

	ph->num_tanks = htonl(ph->num_tanks);
	ph->num_bullets = htonl(ph->num_bullets);
	ph->num_explosions = htonl(ph->num_explosions);

	/* send it to the client */
	if (sendto(port_fd, buf, packet_size, NULL,
		   (struct sockaddr *) &(p->client_addr),
		   sizeof (struct sockaddr_in)) != 
	    packet_size)
	{
	    fprintf(stderr, "Could not send packet to client\n");
	    perror("sendto");
	    exit (1);
	}
    }
    free (buf);
    free (all_packets);
}
