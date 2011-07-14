/* sintab.c - fast table lookup sin/cos */

#include <math.h>

#include "sintab.h"

#define TABLE_MASK 0xff
#define TABLE_SHIFT 10

static int sintab[TABLE_SIZE];

/* 
 * rather than just storing off a set of sin and cos values, we
 * multiply them by (1 << TABLE_SHIFT) and then when we need a radius
 * times some value of sin or cos, we multiply *and then* shift.  This
 * allows us to use only integer arithmetic.
 */

/* return a radius * sin(angle) */
int 
    SIN(angle, rad)
int angle;
int rad;
{
    int val;
    val = (rad * sintab[angle & TABLE_MASK]) >> TABLE_SHIFT;
    return (val < 0 ? val + 1 : val);
}

/* build the table */
void 
    init_sintab()
{
    int i;

    for (i = 0; i < TABLE_SIZE; i++)
    {
        sintab[i] = (int) (((double) (1 << TABLE_SHIFT)) *
                           sin(2.0 * M_PI * ((double) i) /
                               ((double) TABLE_SIZE)));
    }
}
