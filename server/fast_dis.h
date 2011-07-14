/* fast_dis.h - fast sqrt(x*x+y*y) approximator */

/*
 * fast distance estimator.  treats distance as a linear function of
 * x's and y's absolute values.  
 * sqrt (2) is about (1 + 1/2 + 1/16)
 */

#define fast_dis(x, y) (((x) > (y)) ? \
           ((x)+((y)>>1)-((y)>>4)) : \
           ((y)+((x)>>1)-((x)>>4)))
