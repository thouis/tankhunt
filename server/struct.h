/* struct.h - internal structures (except for packets) */

#ifndef STRUCT_H
#define STRUCT_H

#include <netinet/in.h>

#define STAT_ALIVE   0
#define STAT_EXPLODE 1

/* the player structure */
struct player 
{
    int x, y;
    u_char status; /* alive or dead? */
    char speed;
    char des_speed;
    u_char dir;
    u_char des_dir; /* what way we want to go */
    u_char explosion_radius;
    u_char has_bullet;
    int last_ack; /* when we last heard from the client */
    struct sockaddr_in client_addr; /*where they are */

#define NAME_LENGTH 3  /* also in packets.h */
    char name[NAME_LENGTH]; 
};

/* the bullet structure */
struct bullet
{
    int x, y;
    int box_x, box_y; 
    u_char dir;
    u_char status; /* alive or dead? */
    u_char explosion_radius;
};

#endif STRUCT_H
