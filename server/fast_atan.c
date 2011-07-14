/* fast_atan.c - fast atan2 approximator */

#include <sys/types.h>

#include "defs.h"

/* 
 * like fast_dis.h, this treats the atan function as a 
 * linear function of x and y.
 */

/* given an x and y, approximate an atan2 call in the range of 0-255 */
u_char
    fast_atan(x, y)
int x, y;
{
    if (x > ABS(y))
        return(((y << 5) / x) & 255);

    if ((-x) > ABS(y))
        return(128 + (y << 5) / x);

    if (y > ABS(x))
        return(64 - (x << 5) / y);

    if (y)
        return(192 - (x << 5) / y);

    return 0;
}
