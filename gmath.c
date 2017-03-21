#include "gmath.h"
#include "time.h"
#include "stdlib.h"

const double DEG2RAD = 3.141592655358979323846/180.;
unsigned int SEED;

float clamp(float x, float l, float r)
{
	if (x < l)
		return l;
	else if (x > r)
		return r;
	else
		return x;
}

float sign(float x)
{
	return (float) (x > 0) - (x < 0);
}

void normalize(float *v)
{
	float m;
	m = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
	if (m==0)
		return;
	v[0]/=m;
	v[1]/=m;
	v[2]/=m;
}

void cross(float *result, float *v1, float *v2)
{
	result[0] = v1[1]*v2[2] - v1[2]*v2[1];                                      
	result[1] = v1[2]*v2[0] - v1[0]*v2[2];                                      
	result[2] = v1[0]*v2[1] - v1[1]*v2[0];     
}

float lengthdir_x(float len, float dir)
{
	return len*cos(DEG2RAD*dir);
}

float lengthdir_y(float len, float dir)
{
	return len*sin(DEG2RAD*dir);
}

void move_to(float *x, float *y,float dir, float speed)
{
	*x += lengthdir_x(speed,dir);
	*y += lengthdir_y(speed,dir);
}

void seed_rng(unsigned int seed)
{
	SEED = seed;
	srand(seed);
}

void time_seed_rng()
{
	SEED = time(NULL);
	srand(SEED);
}

float random(float range)
{
	SEED++;
	return ((float)rand()/(float)RAND_MAX)*range;
}

int irandom(int range)
{
	SEED++;
	return rand() % range;
}


int choose3(int x1, int x2, int x3)
{
	switch(irandom(3))
	{
		case 0:
			return x1;
		case 1:
			return x2;
		case 2:
			return x3;
	}
	return x1;
}