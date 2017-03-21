#include <SDL2/SDL.h>
#include "state.h"
#include "gmath.h"

static void add_velocity(game_state *state, int entity, float xsp, float ysp, float zsp)
{
	v3 *v = &(state->velocity[entity]);
	v3 *vm = &(state->velocity_max[entity]);

	v->x = (v->x) + xsp;
	v->y = (v->y) + ysp;
	v->z = (v->z) + zsp;
	//printf("vx %f vy %f vz %f\n",v->x,v->y,v->z);

	v->x = clamp(v->x,-vm->x,vm->x);
	v->y = clamp(v->y,-vm->y,vm->y);
	v->z = clamp(v->z,-vm->z,vm->z);
}

static void motion_add(game_state *state, int entity, float dir, float speed)
{
	float xsp = lengthdir_x(speed,dir);
	float ysp = lengthdir_y(speed,dir);
	printf("xsp %f ysp %f\n",xsp,ysp);
	add_velocity(state,entity,xsp,ysp,0);
}

static void move_colliding_with_level(game_state *state, int entity)
{
	v3 *p = &(state->position[entity]);
	v3 *v = &(state->velocity[entity]);

	p->x+=v->x;
	p->y+=v->y;
	p->z+=v->z;
}

static void add_friction(game_state *state, int entity)
{
	v3 *v = &(state->velocity[entity]);
	float f = state->friction[entity];

	if (abs(v->x) > f)
		v->x -= sign(v->x)*f;
	else
		v->x = 0;

	if (abs(v->y) > f)
		v->y -= sign(v->y)*f;
	else
		v->y = 0;

	if (abs(v->z) > f)
		v->z -= sign(v->z)*f;
	else
		v->z = 0;
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

	v3 *vmax = &(state->velocity_max[ent]);
	vmax->x = 16;
	vmax->y = 16;
	vmax->z = 16;

	float *f = &(state->friction[ent]);
	*f = 0.2;

	v3 *bbox = &(state->bbox[ent]);
	bbox->x = 4;
	bbox->y = 4;
	bbox->z = 8;
	return ent;
}

void player_step(game_state *state, const Uint8 *key_state)
{
	// check if player exists
	if (state->player == -1)
		return;
	float spd = 1.0;

	if (key_state[SDL_SCANCODE_W])
		motion_add(state,state->player,state->camdir,spd);
	if (key_state[SDL_SCANCODE_S])
		motion_add(state,state->player,state->camdir+180.f,spd);
	if (key_state[SDL_SCANCODE_D])
		motion_add(state,state->player,state->camdir-90.f,spd);
	if (key_state[SDL_SCANCODE_A])
		motion_add(state,state->player,state->camdir+90,spd);
	if (key_state[SDL_SCANCODE_Q])
		add_velocity(state,state->player,0,0,spd);
	if (key_state[SDL_SCANCODE_E])
		add_velocity(state,state->player,0,0,-spd);
	if (key_state[SDL_SCANCODE_LEFT])
		state->camdir+=1.;
	if (key_state[SDL_SCANCODE_RIGHT])
		state->camdir-=1.;
	if (key_state[SDL_SCANCODE_UP])
		state->camzdir+=1.;
	if (key_state[SDL_SCANCODE_DOWN])
		state->camzdir-=1.;
	if (state->camzdir>89.f)
		state->camzdir = 89.f;
	if (state->camzdir<-89.f)
		state->camzdir = -89.f;
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
	player_step(state,key_state);
	move_colliding_with_level(state,state->player);
	add_friction(state,state->player);
	camera_update(state);

	/*
	float *x,*y,dir;
	x = &(state->camx);
	y = &(state->camy);
	dir = state->camdir;
	if (key_state[SDL_SCANCODE_W])
		move_to(x,y,dir,1.f);
	if (key_state[SDL_SCANCODE_S])
		move_to(x,y,dir+180.f,1.f);
	if (key_state[SDL_SCANCODE_D])
		move_to(x,y,dir-90.f,1.f);
	if (key_state[SDL_SCANCODE_A])
		move_to(x,y,dir+90.f,1.f);
	if (key_state[SDL_SCANCODE_Q])
		state->camz+=1.f;
	if (key_state[SDL_SCANCODE_E])
		state->camz+=-1.f;
	if (key_state[SDL_SCANCODE_LEFT])
		state->camdir+=1.;
	if (key_state[SDL_SCANCODE_RIGHT])
		state->camdir-=1.;
	if (key_state[SDL_SCANCODE_UP])
		state->camzdir+=1.;
	if (key_state[SDL_SCANCODE_DOWN])
		state->camzdir-=1.;
	if (state->camzdir>89.f)
		state->camzdir = 89.f;
	if (state->camzdir<-89.f)
		state->camzdir = -89.f;*/
}