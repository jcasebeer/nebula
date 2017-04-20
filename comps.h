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
	c_sprite,
	c_sprite_fullbright,
	c_gun,
	c_bullet,
	c_last // special enum, keep at end
}component_flag;

typedef struct v3 
{
	float x;
	float y;
	float z;	
}v3;

typedef struct v3i 
{
	int x;
	int y;
	int z;	
}v3i;

typedef struct spr
{
	float sprite_index;
	float image_index;
	float image_count;
	float image_speed;
	float width;
	float height;
	float qwidth;
	float qheight;
	int play_once;
}spr;

typedef struct gun
{
	int active;
	int sprite;
	int type;
	int recoil;
	int rtime;
	float accuracy;
	float speed;
	int damage;
	int range;
	int bullets;
}gun;

#endif