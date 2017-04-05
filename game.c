#include <SDL2/SDL.h>
#include "state.h"
#include "gmath.h"

static void add_velocity(game_state *state, int entity, float xsp, float ysp, float zsp)
{
	v3 *v = &(state->velocity[entity]);

	v->x = (v->x) + xsp;
	v->y = (v->y) + ysp;
	v->z = (v->z) + zsp;
}

static int level_collide(game_state *state, float xx, float yy, float zz, v3i bbox)
{
	int x = (int)xx;
	int y = (int)yy;
	int z = (int)zz;

	return (
		block_at(state,(x-bbox.x)>>5,(y-bbox.y)>>5,(z-bbox.z)>>5) ||
		block_at(state,(x-bbox.x)>>5,(y+bbox.y)>>5,(z-bbox.z)>>5) ||
		block_at(state,(x-bbox.x)>>5,(y-bbox.y)>>5,(z+bbox.z)>>5) ||
		block_at(state,(x-bbox.x)>>5,(y+bbox.y)>>5,(z+bbox.z)>>5) ||
		block_at(state,(x+bbox.x)>>5,(y-bbox.y)>>5,(z-bbox.z)>>5) ||
		block_at(state,(x+bbox.x)>>5,(y+bbox.y)>>5,(z-bbox.z)>>5) ||
		block_at(state,(x+bbox.x)>>5,(y-bbox.y)>>5,(z+bbox.z)>>5) ||
		block_at(state,(x+bbox.x)>>5,(y+bbox.y)>>5,(z+bbox.z)>>5)
	);
}

static void motion_add(game_state *state, int entity, float dir, float speed)
{
	//
	add_velocity(state,entity,lengthdir_x(speed,dir),lengthdir_y(speed,dir),0);
}

static void move_colliding_with_level(game_state *state, int entity)
{
	v3 *p = &(state->position[entity]);
	v3 *v = &(state->velocity[entity]);
	v3i bbox = state->bbox[entity];

	if (!level_collide(state,p->x,p->y,p->z+v->z,bbox))
	{
		p->z+=v->z;
	}
	else
	{
		int z = (float)(p->z);
		if (v->z>0.f)
		{
			float fract = frac(p->z);
			z = ((z+bbox.z+32) & ~31) - bbox.z -1;
			p->z = (float)z + fract;
		}
		else if (v->z<0.f)
		{
			z = ((z-bbox.z) & ~31) + bbox.z;
			p->z = (float)z;
		}
		v->z = 0.f;
	}

	if (!level_collide(state,p->x,p->y+v->y,p->z,bbox))
	{
		p->y+=v->y;
	}
	else
	{
		int y = (int)(p->y);
		if (v->y>0.f)
		{
			float fract = frac(p->y);
			y = ((y+bbox.y+32) & ~31) - bbox.y -1;
			p->y = (float)y + fract;
		}
		else if (v->y<0.f)
		{
			y = ((y-bbox.y) & ~31) + bbox.y;
			p->y = (float)y;
		}
		v->y = 0.f;
	}

	if (!level_collide(state,p->x+v->x,p->y,p->z,bbox))
	{
		p->x+=v->x;
	}
	else
	{
		int x = (int)(p->x);
		if (v->x>0.f)
		{
			float fract = frac(p->x);
			x = ((x+bbox.x+32) & ~31) - bbox.x -1;
			p->x = (float)x + fract;
		}
		else if (v->x<0.f)
		{
			x = ((x-bbox.x) & ~31) + bbox.x;
			p->x = (float)x;
		}
		
		v->x = 0.f;
	}
}

// friction only applied to horizontal movement
// gravity system handles objects falling (vertical movement)
static void add_friction(game_state *state, int entity)
{
	v3 *v = &(state->velocity[entity]);
	float f = state->friction[entity];
	float m = sqrt(v->x * v->x + v->y * v->y);

	if (m!=0)
	{
		float mi = 1/m;

		// normalize vector
		v->x *= mi;
		v->y *= mi;

		m-=f;
		if (m<0)
			m = 0;

		// apply friction
		v->x *= m;
		v->y *= m;	
	}
	
	/*if (abs(v->z) > f)
		v->z -= sign(v->z)*f;
	else
		v->z = 0;*/
}

static void add_gravity(game_state *state, int entity)
{
	v3 *v = &(state->velocity[entity]);
	//v3 *p = &(state->position[entity]);
	v->z-=state->gravity;
}

static void clamp_speed(game_state *state, int entity)
{
	v3 *v = &(state->velocity[entity]);
	float vmax = state->velocity_max[entity];
	float m = sqrt(v->x * v->x + v->y * v->y);
	if (m>vmax)
	{
		float mi = 1/m;
		v->x *= mi;
		v->y *= mi;
		//v->z *= mi;

		v->x *= vmax;
		v->y *= vmax;
		//v->z *= vmax;
	}
	//v->z = clamp(v->z,-vmax,vmax);
}

int player_create(game_state *state, v3 position)
{
	int ent = entity_create(state);

	// set states pointer to the player object
	state->player = ent;
	// give player components
	entity_component_add(state,ent,c_position);
	entity_component_add(state,ent,c_velocity);
	entity_component_add(state,ent,c_velocity_max);
	entity_component_add(state,ent,c_friction);
	entity_component_add(state,ent,c_level_collider);
	entity_component_add(state,ent,c_grounded);
	// initialize components
	state->position[ent] = position;
	v3 *v = &(state->velocity[ent]);
	v->x = 0;
	v->y = 0;
	v->z = 0;

	// velocity and friction
	state->velocity_max[ent] = 2.0;
	state->friction[ent] = 0.2;

	v3i *bbox = &(state->bbox[ent]);
	bbox->x = 6;
	bbox->y = 6;
	bbox->z = 10;
	return ent;
}

int test_sprite(game_state *state,float x, float y, float z)
{
	int ent = entity_create(state);
	entity_component_add(state,ent,c_position);

	v3 *pos = &(state->position[ent]);
	pos->x = x;
	pos->y = y;
	pos->z = z;

	sprite_add(state,ent,0,8,16,16);
	spr *sprite = &(state->sprite[ent]);
	sprite->image_speed = 0.25f;
	return ent;
}

void player_step(game_state *state, const Uint8 *key_state)
{
	// check if player exists
	if (state->player == -1)
		return;
	float spd = 0.3;
	v3 *pos = &(state->position[state->player]);
	v3i bbox = state->bbox[state->player];

	if (key_state[SDL_SCANCODE_W])
		motion_add(state,state->player,state->camdir,spd);
	if (key_state[SDL_SCANCODE_S])
		motion_add(state,state->player,state->camdir+180.f,spd);
	if (key_state[SDL_SCANCODE_D])
		motion_add(state,state->player,state->camdir+90.f,spd);
	if (key_state[SDL_SCANCODE_A])
		motion_add(state,state->player,state->camdir-90.f,spd);

	int grounded = level_collide(state,pos->x,pos->y,pos->z - 1,bbox);

	if (grounded && key_state[SDL_SCANCODE_SPACE])
		add_velocity(state,state->player,0,0,3.0);

	if (key_state[SDL_SCANCODE_R])
	{
		v3 *pos = &(state->position[state->player]);
		test_sprite(state,pos->x,pos->y,pos->z);
	}

	SDL_GetRelativeMouseState(&(state->mouse_x),&(state->mouse_y));

	state->camdir+=(state->mouse_x)/20.f;
	state->camzdir-=(state->mouse_y)/20.f;

	if (key_state[SDL_SCANCODE_LEFT])
		state->camdir-=1.f;
	if (key_state[SDL_SCANCODE_RIGHT])
		state->camdir+=1.f;
	if (key_state[SDL_SCANCODE_UP])
		state->camzdir+=1.f;
	if (key_state[SDL_SCANCODE_DOWN])
		state->camzdir-=1.f;
	if (state->camzdir>88.f)
		state->camzdir = 88.f;
	if (state->camzdir<-88.f)
		state->camzdir = -88.f;
}

static void camera_update(game_state *state)
{
	if (state->player == -1)
		return;
	v3 *pos = &(state->position[state->player]);
	state->camx = pos->x;
	state->camy = pos->y;
	state->camz = pos->z;
}

void game_simulate(game_state *state, const Uint8 *key_state)
{
	int i;
	int *ents;

	// player stuff
	player_step(state,key_state);
	clamp_speed(state,state->player);

	// gravity system
	ents = get_ec_set(state,c_grounded);
	for(i=0; iterate_ec_set(ents,i); i++)
		add_gravity(state,ents[i]);

	// friction system
	add_friction(state,state->player);

	// level collision system
	ents = get_ec_set(state,c_level_collider);
	for(i=0; iterate_ec_set(ents,i); i++)
		move_colliding_with_level(state,ents[i]);

	// camera update system
	camera_update(state);
}
