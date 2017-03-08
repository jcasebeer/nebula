#ifndef GMATH_H
#define GMATH_H
#include <math.h>

extern const double DEG2RAD;
extern unsigned int SEED;
float lengthdir_x(float len, float dir);
float lengthdir_y(float len, float dir);
void normalize(float *v);
void cross(float *result, float *v1, float *v2);
void move_to(float *x, float *y,float dir, float speed);


void seed_rng(unsigned int seed);
void time_seed_rng();
float random(float range);
int irandom(int range);
int choose3(int x1, int x2, int x3);

#endif