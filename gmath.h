#ifndef GMATH_H
#define GMATH_H
#include <math.h>

extern const double DEG2RAD;
float lengthdir_x(float len, float dir);
float lengthdir_y(float len, float dir);
void normalize(float *v);
void cross(float *result, float *v1, float *v2);

#endif