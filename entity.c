#include <stdio.h>
#include <stdlib.h>
#include "state.h"

// create game state object
game_state *game_state_create()
{
	game_state *state = malloc(sizeof(game_state));
	game_state_clear(state);
	printf("size in mb of game_state object: %f\n",(float)sizeof(game_state)/1000000.);
	return state;
}

void game_state_clear(game_state *state)
{
	memset(state,0,sizeof(game_state));
	int i,w;
	for(i=0;i<c_last;i++)
		for(w=0;w<ENTITY_MAX;w++)
		{
			state->comp_lists[i][w] = -1;
			state->has_comp[w][i] = -1;
		}

	state->player = -1;
	state->grapple = -1;
	state->grapple_life = 100;
	state->grapple_state = 0;
	state->can_shoot = 1;
	state->gravity = 0.1;
	state->vheight = 14.f;
	state->jumps = 1;
	state->can_jump = 1;
	state->fov = 120.f;
	state->fov_target = 120.f;
	state->frust_length = 0.f;
	state->grapple_swinging = 0;
}

void game_state_destroy(game_state *state)
{
	free(state);
}

void game_state_print(game_state *state)
{
	int i,w;
	printf("count:%d\n",state->entity_count);
	printf("flags:\n");
	for(w=0;w<ENTITY_MAX;w++)
	{
		printf("Entity %d flags:",w);
		for(i=0;i<c_last;i++)
		{
			printf("%d ",entity_has_component(state,w,i));
		}
		printf("\n");
	}
	/*
	printf("ec_list:\n");
	for(i=0;i<c_last;i++)
	{
		printf("ec_list[%d]:",i);
		for(w=0;w<ENTITY_MAX;w++)
		{
			printf(" %d",state->comp_lists[i][w]);
		}
		printf("\n");
	}*/
}

int entity_create(game_state *state)
{
	/* find the first piece of memory available for use as an entity
	   and return the index to that slot */
	int i;
	for(i=0; i<ENTITY_MAX; i++)
	{
		if (!state->active[i])
		{
			state->active[i] = 1;
			state->entity_count++;
			return i;
		}
	}
	printf("ERROR: Too many entitys!\n");
	exit(1);
}

void entity_destroy(game_state *state, int id)
{
	int i;
	for(i=0; i<c_last; i++)
	{
		if (entity_has_component(state, id, i) )
		{
			entity_component_remove(state,id,i);
		}
	}
	state->active[id] = 0;
	state->entity_count--;
}

// manage entity components
int entity_has_component(game_state *state, int id, component_flag flag)
{
	return (state->has_comp[id][flag] != -1);
}

void entity_kill(game_state *state,int entity)
{
	if (!entity_has_component(state,entity,c_dead))
		entity_component_add(state,entity,c_dead);
}

void entity_component_add(game_state *state, int id, component_flag flag)
{
	/*
		set the component flag and in entity id to true and add
		entity to ec_list for the component flag	
	*/
	if (entity_has_component(state,id,flag))
	{
		printf("ERROR: Entity %d already has component %d!\n",id,flag);
		game_state_print(state);
		exit(1);
	}

	state->comp_lists[flag][state->comp_count[flag]] = id;
	state->has_comp[id][flag] = state->comp_count[flag];
	state->comp_count[flag]++;
}

void entity_component_remove(game_state *state, int id, component_flag flag)
{
	/* 
		set the flag f in entity e to false and 
		remove entity from ec_list for the component flag
	*/
	int last_pos = state->comp_count[flag]-1;
	int last_id = state->comp_lists[flag][last_pos];
	int id_pos = state->has_comp[id][flag];

	state->comp_lists[flag][id_pos] = last_id;
	state->has_comp[last_id][flag] = id_pos;

	state->comp_lists[flag][last_pos] = -1;
	state->has_comp[id][flag] = -1;
	state->comp_count[flag]--;
}

// get array of entitys with a certain component
int *get_ec_set(game_state *state, component_flag component)
{
	return (int *)&(state->comp_lists[component]);
}

void ec_set_swap(game_state *state,component_flag flag, int i, int w)
{
	int ie = state->comp_lists[flag][i];
	int we = state->comp_lists[flag][w];
	
	state->comp_lists[flag][i] = we;
	state->comp_lists[flag][w] = ie;

	int temp = state->has_comp[ie][flag];
	state->has_comp[ie][flag] = state->has_comp[we][flag];
	state->has_comp[we][flag] = temp;
}

// use to iterate over set of entitys
int iterate_ec_set(int *set,int id)
{
	return (id<ENTITY_MAX && set[id]!=-1);
}
