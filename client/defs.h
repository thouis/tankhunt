/* defs.h - #defines for the client */

#ifndef DEFS_H
#define DEFS_H

/* width of the window */
#define WIN_WIDTH 901
#define HALF_WIDTH 450

/* height of the window */
#define WIN_HEIGHT 901
#define HALF_HEIGHT 450 

/* how many pixels wide and high is a box */
#define BOX_SIZE 300

/* 
 * maximum number of players the client can handle being on the screen
 * at one time 
 */
#define MAX_PLAYERS 50 

/* 
 * maximum number of bullets the client can handle being on the screen
 * at one time 
 */
#define MAX_BULLETS 200

/* default sevrer the client connects to */
#define DEFAULT_SERVER "lance"

/* default port the client tries to connect to */
#define DEFAULT_PORT 3333

/* maximum size we expect a server packet to be */
#define MAX_PACKET_SIZE 5000 

/* what environment variable do we store the keymap in? */
#define KEY_MAP_VARIABLE "TANK_HUNT_MAP"

#endif DEFS_H
