#include <stdio.h>
#include <stdlib.h>
#include "state.h"

static void ec_set_add_entity(game_state *state, int id, component_flag flag);
static void ec_set_remove_entity(game_state *state, int id, component_flag flag);
static void entity_set_component_flag(game_state *state,int id,component_flag flag, int value);
static int entity_is_empty(game_state *state, int id);

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

void game_state_print(game_state *state)
{
	int i,w;
	printf("count:%d\n",state->entity_count);
	printf("flags:\n");
	for(w=0;w<ENTITY_MAX;w++)
	{
		printf("Entity %d flags:",w);
		for(i=0;i<COMPONENT_FLAG_SIZE*32;i++)
		{
			printf("%d ",entity_has_component(state,w,i));
			if ( i!=0 && i % 32 == 0)
				printf("\t");
		}
		printf("\n");
	}
	printf("ec_list:\n");
	for(i=0;i<c_last;i++)
	{
		printf("ec_list[%d]:",i);
		for(w=0;w<ENTITY_MAX;w++)
		{
			printf(" %d",state->ec_list[i][w]);
		}
		printf("\n");
	}
}

void game_state_clear(game_state *state)
{
	game_state_destroy(state);
	state = game_state_create();
}

int entity_create(game_state *state)
{
	/* find the first piece of memory available for use as an entity
	   and return the index to that slot */
	int i;
	for(i=0; i<ENTITY_MAX; i++)
	{
		if (entity_is_empty(state,i))
		{
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
	state->entity_count--;
}

// manage entity components
int entity_has_component(game_state *state, int id, component_flag flag)
{
	return (state->flags[id][flag/32] >> (flag%32)) & 0x1;
}

void entity_component_add(game_state *state, int id, component_flag flag)
{
	/*
		set the component flag and in entity id to true and add
		entity to ec_list for the component flag	
	*/
	if (entity_has_component(state,id,flag))
	{
		printf("ERROR: Entity already has component!\n");
		exit(1);
	}
	entity_set_component_flag(state,id,flag,1);
	ec_set_add_entity(state,id,flag);
}

void entity_component_remove(game_state *state, int id, component_flag flag)
{
	/* 
		set the flag f in entity e to false and 
		remove entity from ec_list for the component flag
	*/
	entity_set_component_flag(state,id,flag,0);
	ec_set_remove_entity(state,id,flag);
}

// get array of entitys with a certain component
int *get_ec_set(game_state *state, component_flag component)
{
	return (int *)&(state->ec_list[component]);
}

// use to iterate over set of entitys
int iterate_ec_set(int *set,int id)
{
	return (id<ENTITY_MAX && set[id]!=-1);
}

static void ec_set_add_entity(game_state *state, int id, component_flag flag)
{
	/* add entity id to ec_list[flag] */
	int i;
	for(i=0; i<ENTITY_MAX; i++)
	{
		if (state->ec_list[flag][i] == -1)
		{
			state->ec_list[flag][i] = id;
			return;
		}
	}
	printf("ERROR: Too many entitys!\n");
	exit(1);
}

static void ec_set_remove_entity(game_state *state, int id, component_flag flag)
{
	/* remove entity id from ec_list[flag] */
	int i, entity, last;
	entity = -1;
	last = ENTITY_MAX-1;
	for(i=0; i<ENTITY_MAX; i++)
	{
		if (state->ec_list[flag][i] == id)
			entity = i;
		if (i<ENTITY_MAX-1 && state->ec_list[flag][i+1] == -1)
		{
			last = i;
			break;
		}
	}
	if (entity == -1)
	{
		printf("ERROR: Entity not found in list!\n");
		exit(1);
	}

	state->ec_list[flag][entity] = state->ec_list[flag][last];
	state->ec_list[flag][last] = -1;
}

static void entity_set_component_flag(game_state *state,int id,component_flag flag, int value)
{
	/* set the flag of entity id to value (0 or 1) */
	if (value)
		state->flags[id][flag/32] = (state->flags[id][flag/32]) | (0x1 << (flag%32));
	else
		state->flags[id][flag/32] = (state->flags[id][flag/32]) & ~(0x1 <<(flag%32));
}

static int entity_is_empty(game_state *state, int id)
{
	int i,clear;
	clear = 1;
	for(i=0;i<COMPONENT_FLAG_SIZE;i++)
		if(state->flags[id][i]!=0)
		{
			clear = 0;
			break;
		}
	return clear;
}