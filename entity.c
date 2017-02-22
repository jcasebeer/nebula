#include <stdio.h>
#include <stdlib.h>
#include "state.h"

// create game state object
game_state *game_state_create()
{
	int i,w;
	game_state *state = calloc(1,sizeof(game_state));
	printf("size in bytes of game_state object %lu\n",sizeof(game_state));
	for(i=0;i<c_last;i++)
		for(w=0;w<ENTITY_MAX;w++)
		{
			state->ec_list[i][w] = -1;
		}
	return state;
}

void game_state_destroy(game_state *state)
{
	free(state);
}

/*
void game_state_print(game_state *state);
void game_state_clear(game_state *state);

// create and destroy entitys
int entity_create(game_state *state);
void entity_destroy(game_state *state);

// manage entity components
int entity_has_component(game_state *state, int id, component_flag flag);
int entity_component_add(game_state *state, int id, component_flag flag);
int entity_component_remove(game_state *state, int id, component_flag flag);

// get array of entitys with a certain component
int *get_ec_set(game_state *state, component_flag component);

// use to iterate over set of entitys
int iterate_ec_set(int *set,int id); */