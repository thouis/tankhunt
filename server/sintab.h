/* stinab.h - defines and declarations for the fast table lookup sin/cos */

#define TABLE_SIZE 256

int SIN();

/* cos(theta) = sin(theta + pi) */
#define COS(angle, rad) SIN((angle) + (TABLE_SIZE >> 2), rad)
