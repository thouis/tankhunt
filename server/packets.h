/* packets.h - packet structures and related defines */

/* This file is shared between the server and client */

#define WALL_NUM 25    /* 
			* sqr(dimension of view in boxes +_1)
			* changing this requires that modification 
			* be made to the pad size in packet_header
			*/

#define VIEW_SIZE 3    /* 
			* how many boxxes should be on the side of 
			* the window for the client
			*/

/* the 4 walls we'll be sending */
#define BOTTOM 1
#define RIGHT  2
#define TOP    4
#define LEFT   8

/* 
 * this is what the client gets:
 *
 * struct packet 
 * {
 *     struct packet_header p;
 *     struct packet_tank tanks[p.num_tanks];
 *     struct packet_bullet bullets[p.num_bullets];
 *     struct packet_explosion explosions[p.num_explosions];
 * };
 */
/* the header */
struct packet_header
{
    int seq_number;  
    short rel_x, rel_y;
    unsigned char walls[WALL_NUM];
    unsigned char pad[3];     /* Make sure WALL_NUM % 4 == 0 */
    int num_tanks;
    int num_bullets;
    int num_explosions;
};

/*
 * tanks, bullets and explosions have to remain the same size as each
 * other  
 */
struct packet_to_client
{
    short rel_x, rel_y;
    unsigned char data;
    unsigned char type;
    unsigned char pad1[2];
};

struct packet_tank
{
    short rel_x, rel_y;
    unsigned char dir;
    unsigned char name[3];
};

struct packet_bullet
{
    short rel_x, rel_y;
    unsigned char pad[4];  
};

struct packet_explosion
{
    short rel_x, rel_y;
    unsigned char radius;
    unsigned char pad[3];
};

/* this is what we get from the client */
/* types of packets */
#define C_ACK       1
#define C_FIRE      2
#define C_TURN      3
#define C_TURN_FIRE 4
#define C_SPEED     5
#define C_EXIT      6

/*
 * length of the name we put below the tank. must keep cpacket  the
 * right size.  
 */
#define NAME_LENGTH 3 /* also in struct.h */

/* the data from the client */
struct cpacket 
{
    int type;
    char data;
    char name[NAME_LENGTH]; 
};
