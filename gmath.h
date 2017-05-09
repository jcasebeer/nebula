#ifndef GMATH_H
#define GMATH_H
#include <math.h>
#include "comps.h"

extern const double DEG2RAD;
extern const double RAD2DEG;
extern unsigned int SEED;
float lengthdir_x(float len, float dir);
float lengthdir_y(float len, float dir);
void normalize(float *v);
void cross(float *result, float *v1, float *v2);
void move_to(float *x, float *y,float dir, float speed);
int sphere_collide(v3 *s1, v3 *s2, float s1_radius, float s2_radius);

void seed_rng(unsigned int seed);
void time_seed_rng();
float random(float range);
int irandom(int range);
int choose3(int x1, int x2, int x3);
float sign(float x);
float clamp(float x, float l, float r);
float roundf(float x);
float frac(float x);
float lerp(float a, float b, float f);
int choose2(int x1, int x2);
void itoa(int num, char *buff);

#endif