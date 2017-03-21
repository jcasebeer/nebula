#ifndef COMPS_H
#define COMPS_H

typedef enum
{
	c_dead,
	c_position,
	c_velocity,
	c_velocity_max,
	c_friction,
	c_grounded,
	c_level_collider, // needs bbox
	c_last // special enum, keep at end
}component_flag;

typedef struct v3 
{
	float x;
	float y;
	float z;	
}v3;

#endif