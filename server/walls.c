/* walls.c - build maze and interface to the walls in the maze */

#include <stdio.h>
#include <sys/types.h>

#include "defs.h"
#include "data.h"
#include "packets.h"
#include "fast_dis.h"

/* the wall storage */
u_char walls[MAX_ARENA][MAX_ARENA];
#define ARENA_DIM (arena_box_dim - 2 * ARENA_OFFSET)

/* we have some extra space in the walls struct area, might as well use it */
/* a temporary constant, used to generate the maze. */
#define HAVE_BEEN 16 

/* the value if all the walls are set */
#define ALL_WALLS (BOTTOM | TOP | LEFT | RIGHT)

/*
 * prob_to_break is (100 - density) and is the probability we'll break 
 * a wall on our way back out on the recursion in connect_squares.  
 * 0 means never break a wall, 100 means break every one of them (big 
 * empty maze).
 */
static int prob_to_break;

/*
 * turn off a certain wall
 */
void
    break_wall(x, y, wall)
int x, y;
u_char wall;
{
    walls[(x + ARENA_DIM) % ARENA_DIM][(y + ARENA_DIM) % ARENA_DIM] &=
	(~ wall);
}

/* 
 * return the value of a location, taking into account the possibility
 * of out of bounds values.
 */
static u_char 
    get_walls(x, y)
int x, y;
{
    return (walls[(x + ARENA_DIM) % ARENA_DIM]
	    [(y + ARENA_DIM) % ARENA_DIM]);
}

/* 
 * recursize function to generate the arena.  generates it in the
 * lower left part of the array, later shifted to the center by
 * gen_maze.
 */
void 
    connect_squares(x, y, unmask)
int x, y;
u_char unmask;
{
    int i = 0;

    x = (x + ARENA_DIM) % ARENA_DIM;
    y = (y + ARENA_DIM) % ARENA_DIM;

    walls[x][y] |= HAVE_BEEN;
    walls[x][y] ^= unmask;

    while (1)
    {
	if ((get_walls(x + 1, y) & HAVE_BEEN) &&
	    (get_walls(x - 1, y) & HAVE_BEEN) &&
	    (get_walls(x, y + 1) & HAVE_BEEN) &&
	    (get_walls(x, y - 1) & HAVE_BEEN))
	{
	    u_char which_wall;

	    if ((random() % 100) < prob_to_break)
	    {
		break_wall(x, y, TOP);
		break_wall(x, y + 1, BOTTOM);
	    }
	    if ((random() % 100) < prob_to_break)
	    {
		break_wall(x, y, BOTTOM);
		break_wall(x, y - 1, TOP);
	    }
	    if ((random() % 100) < prob_to_break)
	    {
		break_wall(x, y, RIGHT);
		break_wall(x + 1, y, LEFT);
	    }
	    if ((random() % 100) < prob_to_break)
	    {
		break_wall(x, y, LEFT);
		break_wall(x - 1, y, RIGHT);
	    }
	    return;
	}

	switch (random() % 4)
	{
	case 0:
	    if (get_walls(x + 1, y) & HAVE_BEEN)
		break;
	    walls[x][y] ^= RIGHT;
	    connect_squares(x + 1, y, LEFT);
	    break;
	case 1:
	    if (get_walls(x - 1, y) & HAVE_BEEN)
		break;
	    walls[x][y] ^= LEFT;
	    connect_squares(x - 1, y, RIGHT);
	    break;
	case 2:
	    if (get_walls(x, y + 1) & HAVE_BEEN)
		break;
	    walls[x][y] ^= TOP;
	    connect_squares(x, y + 1, BOTTOM);
	    break;
	case 3:
	    if (get_walls(x, y - 1) & HAVE_BEEN)
		break;
	    walls[x][y] ^= BOTTOM;
	    connect_squares(x, y - 1, TOP);
	    break;
	}
    }
}

/* 
 * given a value in the fringe, return the value of its center
 * counterpart. 
 */
u_char 
    mirror_walls(x, y)
{
    x = ((x - ARENA_OFFSET + ARENA_DIM) % ARENA_DIM) + ARENA_OFFSET;
    y = ((y - ARENA_OFFSET + ARENA_DIM) % ARENA_DIM) + ARENA_OFFSET;

    return (walls[x][y]);
}

/* these corner values are used when testing for wall collisions */
#define TOP_RIGHT    16
#define TOP_LEFT     32
#define BOTTOM_RIGHT 64
#define BOTTOM_LEFT  128

/*
 * generate the maze, then build the boundary of the arena (see
 * defs.h), then put in the corner values so that testing for
 * collisions is faster/easier. 
 */
void 
    gen_walls(density)
int density;
{
    int i, j;

    /* clear the array */
    for (i = 0; i < arena_box_dim; i++)
	for (j = 0; j < arena_box_dim; j++)
	{
	    walls[i][j] = ALL_WALLS;
	}
    /* create the maze */
    prob_to_break = 100 - density;
    connect_squares(0, 0, 0);
    fprintf(stderr, "Done generating maze.\n");

    /* shift the arena to the center of the array */
    for (i = 0; i < ARENA_DIM; i++)
	for (j = 0; j < ARENA_DIM; j++)
	{
	    walls[ARENA_DIM + ARENA_OFFSET - i - 1]
		[ARENA_DIM + ARENA_OFFSET - j - 1] = 
		    walls[ARENA_DIM - i - 1][ARENA_DIM - j - 1] & ALL_WALLS;
	}

    /* Build the outside boundary */
    for (i = 0; i < ARENA_OFFSET; i++)
	for (j = 0; j < arena_box_dim; j++)
    {
	walls[i][j] = mirror_walls(i, j);
	walls[j][i] = mirror_walls(j, i);
    }
    for (i = ARENA_DIM + ARENA_OFFSET; i < arena_box_dim; i++)
	for (j = 0; j < arena_box_dim; j++)
    {
	walls[i][j] = mirror_walls(i, j);
	walls[j][i] = mirror_walls(j, i);
    }
    
    /* 
     * after we have mirrored the walls, put in the corner values.
     * This only needs to be done for the internal sqquares (never
     * have to test on the fringe.  a corner is set if one of the the
     * walls of that box are set near that corner, or if one of the
     * walls opposite that corner is set.  it's faster to do this in
     * only one loop (checking just one square into the fringe) but
     * this is easier to understand IMO.
     */

    /* check internal walls for all the squares */
    for (i = 0; i < arena_box_dim; i++)
	for ( j = 0; j < arena_box_dim; j++)
	{
	    if (walls[i][j] & TOP)
		walls[i][j] |= (TOP_RIGHT | TOP_LEFT);

	    if (walls[i][j] & BOTTOM)
		walls[i][j] |= (BOTTOM_RIGHT | BOTTOM_LEFT);

	    if (walls[i][j] & LEFT)
		walls[i][j] |= (TOP_LEFT | BOTTOM_LEFT);

	    if (walls[i][j] & RIGHT)
		walls[i][j] |= (TOP_RIGHT | BOTTOM_RIGHT);
	}

    /* check external corners for the center only */
    for (i = ARENA_OFFSET; i < ARENA_DIM + ARENA_OFFSET; i++)
	for ( j = ARENA_OFFSET; j < ARENA_DIM + ARENA_OFFSET; j++)
	{
	    if (walls[i + 1][j + 1] & BOTTOM_LEFT)
		walls[i][j] |= TOP_RIGHT;

	    if (walls[i - 1][j - 1] & TOP_RIGHT)
		walls[i][j] |= BOTTOM_LEFT;

	    if (walls[i - 1][j + 1] & BOTTOM_RIGHT)
		walls[i][j] |= TOP_LEFT;

	    if (walls[i + 1][j - 1] & TOP_LEFT)
		walls[i][j] |= BOTTOM_RIGHT;
	}
}

/*
 * test if a location in the arena (box_x, box_y) has a wall near
 * (rel_x, rel_y) in that box.  
 * BOX_SIZE > rel_x > 0, BOX_SIZE > rel_y > 0 
 * returns true if there is one.
 */
u_char
    touching_wall(box_x, box_y, rel_x, rel_y)
int box_x, box_y;
int rel_x, rel_y;
{
    u_char local_walls;
    int x_prime, y_prime;

    /* don't forget to move to the center */
    local_walls = walls[box_x + ARENA_OFFSET][box_y + ARENA_OFFSET];

    x_prime = BOX_SIZE - rel_x;
    y_prime = BOX_SIZE - rel_y;

    if (rel_x < TANK_RADIUS)
    {
	if (fast_dis(rel_x, rel_y) < TANK_RADIUS)
	    return (local_walls & BOTTOM_LEFT);

	if (fast_dis(rel_x, y_prime) < TANK_RADIUS)
	    return (local_walls & TOP_LEFT);

	if (rel_y < TANK_RADIUS)
	    return (local_walls & BOTTOM);

	if (y_prime < TANK_RADIUS)
	    return (local_walls & TOP);

	return (local_walls & LEFT);
    }

    if (x_prime < TANK_RADIUS)
    {
	if (fast_dis(x_prime, rel_y) < TANK_RADIUS)
	    return (local_walls & BOTTOM_RIGHT);

	if (fast_dis(x_prime, y_prime) < TANK_RADIUS)
	    return (local_walls & TOP_RIGHT);

	if (rel_y < TANK_RADIUS)
	    return (local_walls & BOTTOM);

	if (y_prime < TANK_RADIUS)
	    return (local_walls & TOP);

	return (local_walls & RIGHT);
    }

    if (rel_y < TANK_RADIUS)
	return (local_walls & BOTTOM);
    
    if (y_prime < TANK_RADIUS)
	return (local_walls & TOP);

    return (0);
}

/*
 * bcopy the wall data surrounding a location in the arena to a
 * destination.  Since walls are stored the same way they are
 * communicated, this is suitable for sending directly to the client.
 * returns true if we hit a wall.
 */
u_char
    fetch_wall_data(box_x, box_y, rel_x, rel_y, size, out)
int box_x, box_y;
int rel_x, rel_y;
int size;
char *out;
{
    int i;

    /* 
     * we add ARENA_OFFSET to the box_x and box_y locations in this
     * next call (to shift it into the centered arena, but then we
     * subtract ARENA_OFFSET again because we want to send the data
     * centered around the tank.  In either case, it ends up dropping
     * out of the equation.
     */
    for (i = 0; i < size; i++, out += size)
	bcopy(&(walls[box_x + i][box_y]), out, size);

    return (touching_wall(box_x, box_y, 
			  rel_x, rel_y));
}

/* 
 * return the wall data for just one box, at x, y.
 */
u_char 
    get_one_wall(x, y)
int x, y;
{
    /* don't forget to shift to center */
    return (walls[x + ARENA_OFFSET][y + ARENA_OFFSET]);
}
