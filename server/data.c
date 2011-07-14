/* data.c - declarations for globals */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "defs.h"
#include "struct.h"

int port_fd; /* the file descriptor for the UDP port we open */

int sequence_number = 0; /* 
			  * global sequence number, sent out with
			  * update packets.  This allows the client to
			  * throw away old packets.
			  */

int num_players = 0; /* number of players currently in the game */
int num_bullets = 0; /* number of bullets that are currently alive */

struct player players[MAX_PLAYERS]; /* the player array */
struct bullet bullets[MAX_PLAYERS]; /* the bullet array */

/* how big the arena is.  multiple of BOX_SIZE */
int arena_size = 10 * BOX_SIZE; 

/* half the arena, used when computing relative positions between tanks */
int half_arena = 5 * BOX_SIZE; 

/*
 * This value is sort of magic.  We want to be able to wrap around at
 * the boundaries of the arena, so we need to be able to see past the
 * real boundaries.  To do this, we need to put some boxes around the
 * boundary of the arena, mirroring the boxes on the opposite side of
 * the arena. 
 * To get the value : (arena_size / BOX_SIZE) +
 *                    2 * ceil(VIS_DISTANCE / BOX_SIZE)
 */
int arena_box_dim = 14;
