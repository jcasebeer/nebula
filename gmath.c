#include "gmath.h"
#include "time.h"
#include "stdio.h"
#include "stdlib.h"
//#include "comps.h"

const double DEG2RAD = 3.141592655358979323846/180.;
const double RAD2DEG = 180.f/3.141592655358979323846;
unsigned int SEED;

void compliment(float color[4], float result[4])
{
	result[0] = (1.f - color[0])/2.f;
	result[1] = (1.f - color[1])/2.f;
	result[2] = (1.f - color[2])/2.f;
	result[3] = 1.f;
}

v3 v3_create(float x, float y, float z)
{
	v3 v;
	v.x = x;
	v.y = y;
	v.z = z;
	return v;
}

float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

float clamp(float x, float l, float r)
{
	if (x < l)
		return l;
	else if (x > r)
		return r;
	else
		return x;
}

float roundf(float x)
{
	return floor(x+0.5);
}

float frac(float x)
{
	return x - floor(x);
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
	return -len*sin(DEG2RAD*dir);
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
int choose2(int x1, int x2)
{
	switch(irandom(2))
	{
		case 0:
			return x1;
		case 1:
			return x2;
	}
	return x1;
}

int sphere_collide(v3 *s1, v3 *s2, float s1_radius, float s2_radius)
{
	float xdiff = s1->x - s2->x;
	float ydiff = s1->y - s2->y;
	float zdiff = s1->z - s2->z;
	float rsum = s1_radius+s2_radius;

	//return sqrt(xdiff*xdiff + ydiff*ydiff + zdiff*zdiff) < rsum
	return (xdiff*xdiff + ydiff*ydiff + zdiff*zdiff) < rsum*rsum;
}

v3 dirToVector(float dir, float zdir, float m)
{
	v3 v;
	v.x = lengthdir_x(lengthdir_x(1,-zdir),dir);
	v.y = lengthdir_y(lengthdir_x(1,-zdir),dir);
	v.z = lengthdir_y(1,-zdir);

	float dp = v.x*v.x + v.y*v.y + v.z*v.z;
	float im = 1.f;
	if (dp != 0.f)
		im = m/dp;
	v.x*=im;
	v.y*=im;
	v.z*=im;
	
	return v;
}

void itoa(int num, char *buff)
{
	const char *dig = "0123456789";
	int div = 1;
	int c = 0;
	if (num == 0)
	{
		buff[0] = '0';
		buff[1] = '\0';
		return;
	}
	while(num)
	{
		num/=div;
		div=10;
		int n = (num % 10);
		if (num!=0)
		{
			buff[c] = dig[n];
			c++;
		}
	}
	c--;
	for(int i = 0; i<=c/2; i++)
	{
		int temp = buff[c-i];
		buff[c-i] = buff[i];
		buff[i] = temp;
	}
	buff[++c] = '\0';
}
