#ifndef STATE_H
#define STATE_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "comps.h"

typedef struct persistent_state
{
	int weapon;
	int grapple_out;
	gun weapons[2];
}p_state;

typedef struct game_state
{
	p_state *pstate;
	#define ENTITY_MAX 512 // max number of entitys
	/* running total number of entitys*/
	int entity_count;

	#define COMPONENT_FLAG_SIZE 1 // max component flags = COMPONENT_FLAG_SIZE*32
	/* bitmask of flags per entity */
	int flags[ENTITY_MAX][COMPONENT_FLAG_SIZE];

	/* data structure with lists of entity ids with a given component
	   ex: ec_list[c_none] is the array of all the entitys with the c_none component flag */
	int ec_list[c_last][ENTITY_MAX];

	/* player data */
	// player entity id
	int player;
	int grapple;
	int grapple_life;
	int grapple_state;
	float vheight;
	float view_bob;
	float jumps;
	float can_jump;
	float can_shoot;
	float gun_change;
	float in_shadow;
	/* camera vars */
	float camx;
	float camy;
	float camz;
	float camdir;
	float camzdir;
	float gundir;
	float gunzdir;
	float cam_shake;
	int mouse_x;
	int mouse_y;
	int mouse_rb;
	int mouse_lb;
	int timer;
	int frame_time;

	/* component arrays */
	v3 position[ENTITY_MAX];
	v3 velocity[ENTITY_MAX];
	float velocity_max[ENTITY_MAX];
	float friction[ENTITY_MAX];
	int damage[ENTITY_MAX];
	int life[ENTITY_MAX];
	float radius[ENTITY_MAX];
	v3i bbox[ENTITY_MAX];
	spr sprite[ENTITY_MAX];
	gun guns[ENTITY_MAX];

	/* level data */
	#define LEVEL_SIZE 256
	#define MAX_BLOCKS 130000
	#define BLOCK_SIZE 32
	int block_grid[LEVEL_SIZE][LEVEL_SIZE][LEVEL_SIZE];
	int block_list[MAX_BLOCKS];
	int block_count;
	int next_level;
	GLfloat levelColor[4];
	// gl index to display list for level model
	GLuint level_model;
	GLuint grass_model;
	GLuint dust_model;
	float dust_anim;
	float gravity;
}game_state;

p_state *p_state_create();

// create game state object
game_state *game_state_create();
void game_state_destroy(game_state *state);
void game_state_print(game_state *state);
void game_state_clear(game_state *state);

// create and destroy entitys
int entity_create(game_state *state);
void entity_destroy(game_state *state, int id);
void entity_kill(game_state *state,int entity);

// manage entity components
int entity_has_component(game_state *state, int id, component_flag flag);
void entity_component_add(game_state *state, int id, component_flag flag);
void entity_component_remove(game_state *state, int id, component_flag flag);

// get array of entitys with a certain component
int *get_ec_set(game_state *state, component_flag component);

// use to iterate over set of entitys
int iterate_ec_set(int *set,int id);

/****** LEVEL STUFF **************/
void level_gen(game_state *state);
void level_next(game_state *state, int clearModels, p_state *pstate);
int point_getx(int block);
int point_gety(int block);
int point_getz(int block);
int point_create(int x, int y, int z);

int block_at(game_state *state,int x, int y, int z);
void block_create(game_state *state, int x, int y, int z);
int block_at_bounded(game_state *state, int x, int y, int z);
int block_get_lit(game_state *state,int x, int y, int z);

/***** gameplay stuff *********/
void game_simulate(game_state *state,const Uint8 *key_state);
gun gen_gun();

/***** entitys *******/
int player_create(game_state *state, v3 position);

/***** component adds ******/
void sprite_add(game_state *state, int entity, float sprite_index, float image_count, float width, float height);
void sprite_add_size(game_state *state, int entity, float sprite_index,float image_count, float width, float height, float qwidth, float qheight);

#endif