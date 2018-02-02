#ifndef STATE_H
#define STATE_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "comps.h"
#include "sound.h"

// input macros, require key_state and prev_key_state pointers in scope
#define key_pressed(key) (key_state[key] && !prev_key_state[key])
#define key_down(key) (key_state[key])

// stuff you want to stay persistent between levels
typedef struct
{
	int weapon;
	int grapple_out;
	gun weapons[2];
}p_state;

typedef struct game_state
{
	p_state pstate;
	#define ENTITY_MAX 256 // max number of entitys
	/* running total number of entitys*/
	int entity_count;

	/* whether entity is actived or not*/
	int active[ENTITY_MAX];

	/* how many components of a type exist */
	int comp_count[c_last];

	/* arrays of entitys organized by which components they have */
	int comp_lists[c_last][ENTITY_MAX];

	/* sparse array indicating whether an entity has a component
	   stores the location of the given entity id in comp_lists
	   stores -1 for an entity without that component */
	int has_comp[ENTITY_MAX][c_last];

	/* player data */
	// player entity id
	int player;
	int grapple;
	int grapple_life;
	int grapple_swinging;
	int grapple_state;
	float grapple_dist;
	float vheight;
	float view_bob;
	float jumps;
	float can_jump;
	float can_shoot;
	float gun_change;
	float in_shadow;
	float fov;
	float fov_target;
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
	pLight lights[ENTITY_MAX];

	/* level data */
	#define LEVEL_SIZE 256
	#define MAX_BLOCKS 500000
	#define BLOCK_SIZE 32
	#define CHUNK_SIZE 250
	#define DELETE_QUEUE_SIZE 250

	int block_destroyer;

	// 1 bit per block/air block
	int block_grid[LEVEL_SIZE*LEVEL_SIZE*LEVEL_SIZE/BLOCK_SIZE +1];
	int block_list[MAX_BLOCKS];
	int delete_queue[DELETE_QUEUE_SIZE];
	int dq_front;
	int dq_end;
	int block_count;
	int next_level;
	GLfloat levelColor[4];
	GLfloat levelFogColor[4];
	GLfloat levelGrassColor[4];
	// gl index to display list for level model
	GLuint level_model[MAX_BLOCKS/CHUNK_SIZE];
	int level_model_count;
	GLuint grass_model[MAX_BLOCKS/CHUNK_SIZE];
	GLuint dust_model;
	float dust_anim;
	float day_night;
	float gravity;
	float frust_length;
}game_state;

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
void ec_set_swap(game_state *state, component_flag flag, int i, int w);

// use to iterate over set of entitys
int iterate_ec_set(int *set, int id);

/****** LEVEL STUFF **************/
void level_gen(game_state *state);
void level_next(game_state *state, int clearModels);
int point_getx(int block);
int point_gety(int block);
int point_getz(int block);
int point_create(int x, int y, int z);
void bit_clear(game_state *state, int x, int y, int z);
int light_create(game_state *state, v3 position, float radius);
void delete_queue_enter(game_state *state, int point);
int delete_queue_remove(game_state *state);
int delete_queue_is_empty(game_state *state);
int delete_queue_has(game_state *state, int block);
int block_at(game_state *state, int x, int y, int z);
void block_create(game_state *state, int x, int y, int z);
int block_at_bounded(game_state *state, int x, int y, int z);
int block_get_lit(game_state *state, int x, int y, int z);

/***** gameplay stuff *********/
void game_simulate(game_state *state, const Uint8 *key_state, Uint8 *prev_key_state);
gun gen_gun();
v3 v3_create(float x, float y, float z);

/***** entitys *******/
int player_create(game_state *state, v3 position);

/***** component adds ******/
void sprite_add(game_state *state, int entity, float sprite_index, float image_count, float width, float height);
void sprite_add_size(game_state *state, int entity, float sprite_index,float image_count, float width, float height, float qwidth, float qheight);

#endif