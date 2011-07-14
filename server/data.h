/* data.h - external declarations for the globals.  see data.c */

#ifndef DATA_H
#define DATA_H

#include "struct.h"

extern int port_fd;

extern int sequence_number;

extern int num_players;
extern int num_bullets;

extern struct player players[MAX_PLAYERS];
extern struct bullet bullets[MAX_PLAYERS];

extern int arena_size;
extern int half_arena;
extern int arena_box_dim;

#endif
