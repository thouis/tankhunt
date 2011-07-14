/* defs.h - commonly used macros/defines */

#ifndef DEFS_H
#define DEFS_H

#define DEFAULT_PORT 3333 /* default port for the server */

#define MAX_PLAYERS 100 /* this may be high.  tests need to be run */

#define UPDATE 100000 /* number of milliseconds to wait between updates */

#define TIMEOUT 600 /* after this many updates without hearing from a
		     * player, assume they are dead. 
		     */

#define TURN_RATE 10 /* this is the maximum a tank can turn during a
		      * single update.
		      */

/* the largest an arena can be, including boundary */
#define MAX_ARENA 104

#define BOX_SIZE 300    /* size of a box in the arena */

#define VIS_DISTANCE 450   /* how far can I see?  1.5 boxes, default */
/*
 * how wide is the boundary?  This is also how far we need to shift from 
 * the tank's center when sending the surounding boxes.
 * ceil(VIS_DISTANCE / BOX_SIZE) 
 */
#define ARENA_OFFSET  2

#define SPEED_SCALE 3 /* we scale the client's speed by this amount
		       * when actually moving 
		       */
#define MAX_SPEED 8 /* the maximum speed we will accept from the client. */

#define BULLET_SPEED 40 /* how fast do bullets move. (not affected by
			 * SPEED_SCALE 
			 */

#define TANK_RADIUS 32 /* How big are tanks? */
#define COLLISION_DISTANCE 64  /* 2 * TANK_RADIUS */

#define MAX_EXPLOSION_RADIUS 64 /* tanks explode until they are this big. */
#define MAX_BULLET_EXPLOSION 32 /* bullets "      "     "    "   "    "   */

#define EXPLOSION_SPEED 5 /* how many units does an explosion's radius
			   * expand by per update? 
			   */

#define DEFAULT_DENSITY 97 /* how dense should the maze be by default */

/* useful function */
#define ABS(x) (((x)>0)?(x):-(x))

#endif DEFS_H
