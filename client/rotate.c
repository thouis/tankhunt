/* rotate.c - generate rotations of a bitmap */

#include <sys/types.h>
#include <math.h>

#define MAX_SIZE 256

static intern_src[MAX_SIZE][MAX_SIZE];
static intern_dest[MAX_SIZE][MAX_SIZE];

/* these functions both convert from bits to characters and flip */

/* convert and flip so the bitmap faces to the left */
void 
    flip_to_0(bits, size)
u_char *bits;
int size;
{
    int x, y;
    int count = 1;

    for (y = 0; y < size; y++)
	for (x = 0; x < size; x++)
	{
	    intern_src[y][x] = *bits & count;
	    count <<= 1;
	    if (count > 128)
	    {
		count = 1;
		bits++;
	    }
	}
}

/* flip to facing up */
void 
    flip_to_64(bits, size)
u_char *bits;
int size;
{
    int x, y;
    int count = 1;

    for (y = 0; y < size; y++)
	for (x = 0; x < size; x++)
	{
	    intern_src[size - x - 1][y] = *bits & count;
	    count <<= 1;
	    if (count > 128)
	    {
		count = 1;
		bits++;
	    }
	}
}

/* flip to facing left */
void 
    flip_to_128(bits, size)
u_char *bits;
int size;
{
    int x, y;
    int count = 1;

    for (y = 0; y < size; y++)
	for (x = 0; x < size; x++)
	{
	    intern_src[size - y - 1][size - x - 1] = *bits & count;
	    count <<= 1;
	    if (count > 128)
	    {
		count = 1;
		bits++;
	    }
	}
}

/* flip to facing down */
void 
    flip_to_192(bits, size)
u_char *bits;
int size;
{
    int x, y;
    int count = 1;

    for (y = 0; y < size; y++)
	for (x = 0; x < size; x++)
	{
	    intern_src[x][size - y - 1] = *bits & count;
	    count <<= 1;
	    if (count > 128)
	    {
		count = 1;
		bits++;
	    }
	}
}

/*
 * Rotate a bitmap (size on a side) some angle (0 - 255).
 */
void
    rotate_bitmap(angle, bits_in, bits_out, size)
u_char angle;
u_char *bits_in, *bits_out;
int size;
{
    /* 
     * To get the best quality in rotations, we should rotate the
     * shortest amount possible.  To do this, we use flips to get
     * the North, East, West and South rotations.  (We're given the
     * East, actually.)  After getting one of these rotations, we
     * rotate some small amount (always less than PI / 4, in one
     * direction or the other.)
     * We unpack the array into an array of booleans at the same time
     * as we do the flip. 
     */
    
    u_char quadrant;
    double real_angle, sinval, cosval;
    int x, y;
    int x_prime, y_prime;
    int shift = size >> 1;
    u_char *bits, count;

    /* Which quadrant are we nearest? */
    /* add 32 and divide by 64 */
    quadrant = angle + 32;
    quadrant >>= 6;
    switch (quadrant)
    {
    case 0:
	/* angle * 2 * PI / 256 */
	real_angle = ((double) angle) * M_PI / 128.0;
	flip_to_0(bits_in, size); /* not really a flip */
	break;
    case 1:
	real_angle = ((double) (angle - 64)) * M_PI / 128.0;
	flip_to_64(bits_in, size);
	break;
    case 2:
	real_angle = ((double) (angle - 128)) * M_PI / 128.0;
	flip_to_128(bits_in, size);
	break;
    case 3:
	real_angle = ((double) (angle - 192)) * M_PI / 128.0;
	flip_to_192(bits_in, size);
	break;
    }

    if (real_angle == 0.0)
    {
	for (y = 0; y < size; y++)
	    for (x = 0; x < size; x++)
		intern_dest[y][x] = intern_src[y][x];
    }
    else
    {
	/* we don't want to call sin() and cos() more than necessary */
	sinval = sin(real_angle);
	cosval = cos(real_angle);

	for (y = -shift; y < shift; y++)	
	    for (x = -shift; x < shift; x++)
	    {
		x_prime = ((int) ((double) x) * cosval - 
			   ((double) y) * sinval) +
			       shift;
		y_prime = ((int) ((double) y) * cosval + 
			   ((double) x) * sinval) +
			       shift;
		if ((x_prime < 0) || (x_prime >= size) ||
		    (y_prime < 0) || (y_prime >= size))
		    intern_dest[y + shift][x + shift] = 0;
		else
		    intern_dest[y + shift][x + shift] = 
			intern_src[y_prime][x_prime];
	    }
    }

    bits = bits_out;
    count = 1;
    for (y = 0; y < size; y++)
	for (x = 0; x < size; x++)
	{
	    *bits |= (intern_dest[y][x] ? count : 0);
	    if (count == 128)
	    {
		count = 1;
		bits++;
		*bits = 0;
	    }
	    else
		count <<= 1;
	}
}
