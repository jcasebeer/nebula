#ifndef STATE_H
#define STATE_H

#include "comps.h"

typedef struct game_state
{
	#define ENTITY_MAX 512 // max number of entitys
	/* running total number of entitys*/
	int entity_count;

	#define COMPONENT_FLAG_SIZE 1 // max component flags = COMPONENT_FLAG_SIZE*32
	/* bitmask of flags per entity */
	int flags[ENTITY_MAX][COMPONENT_FLAG_SIZE];

	/* data structure with lists of entity ids with a given component
	   ex: ec_list[c_none] is the array of all the entitys with the c_none component flag */
	int ec_list[c_last][ENTITY_MAX];

	/* component arrays */
	v3 position[ENTITY_MAX];
	v3 velocity[ENTITY_MAX];

	/* level data */
	#define LEVEL_SIZE 192
	#define MAX_BLOCKS 32768
	#define BLOCK_SIZE 32
	int block_grid[LEVEL_SIZE][LEVEL_SIZE][LEVEL_SIZE];
	int block_list[MAX_BLOCKS];
	int block_count;

}game_state;

// create game state object
game_state *game_state_create();
void game_state_destroy(game_state *state);
void game_state_print(game_state *state);
void game_state_clear(game_state *state);

// create and destroy entitys
int entity_create(game_state *state);
void entity_destroy(game_state *state, int id);

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
int point_getx(int block);
int point_gety(int block);
int point_getz(int block);
int point_create(int x, int y, int z);

#endif