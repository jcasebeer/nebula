#include <stdio.h>
#include <stdlib.h>
#include "render.h"
#include "state.h"
#include "gmath.h"

static void bit_set(game_state *state, int x, int y, int z)
{
	int i = ((x*LEVEL_SIZE)+y)*LEVEL_SIZE+z;
	state->block_grid[i>>5] = state->block_grid[i>>5] | (1 << i%32);
}

void bit_clear(game_state *state, int x, int y, int z)
{
	int i = ((x*LEVEL_SIZE)+y)*LEVEL_SIZE+z;
	state->block_grid[i>>5] = state->block_grid[i>>5] & ~(1 << i%32);
}

static int bit_get(game_state *state, int x, int y, int z)
{
	int i = ((x*LEVEL_SIZE)+y)*LEVEL_SIZE+z;
	return (state->block_grid[i>>5] >> (i%32)) & 1;
}

void level_gen(game_state *state)
{
	/*
	//add colors to this array as we gind good ones
	
	#define COLORCOUNT 3
	v3i colors[COLORCOUNT] = {
		{255,128,128},
		{128,255,128},
		{128,128,255}
	};
	int color = irandom(COLORCOUNT); */

    float hue = -30.f+random(180.f);
	hsv_to_rgb(hue,0.3+random(0.4),1.0,state->levelColor);
	hsv_to_rgb(hue,0.1,0.9,state->levelGrassColor);
	hue-=180.f;
	hsv_to_rgb(hue,0.35,0.30,state->levelFogColor);


	int blocks = LEVEL_SIZE * BLOCK_SIZE * 4;
	int x,y,z;
	int i;

	// set all initial values in block_grid to -1
	int *gptr = &(state->block_grid[0]);
	int block_grid_size = LEVEL_SIZE*LEVEL_SIZE*LEVEL_SIZE/BLOCK_SIZE;
	for(i=0;i<block_grid_size;i++)
	{
		*(gptr+i) = 0;
	}

	state->block_count = 0;
	/*for (x=0; x<LEVEL_SIZE; x++)
		for(y=0; y<LEVEL_SIZE; y++)
		{
			block_create(state,x,y,idist2d(x,y,LEVEL_SIZE>>1,LEVEL_SIZE>>1));
		}*/
	
	x = LEVEL_SIZE / 2;
	y = x;
	z = x;

	int xstart, ystart, zstart;
	xstart = x;
	ystart = y;
	zstart = z; 
	load_screen_draw(15);

	while(blocks>0)
	{
		if (!bit_get(state,x,y,z))
		{
			block_create(state,x,y,z);
			blocks--;
		}
		x+=choose3(1,0,-1);
		y+=choose3(1,0,-1);
		z+=choose3(1,0,-1);
		while (x>=LEVEL_SIZE || x < 0 || y>=LEVEL_SIZE || y < 0 || z>=LEVEL_SIZE || z < 0)
		{
			int point = state->block_list[irandom(state->block_count)];
			x = point_getx(point);
			y = point_gety(point);
			z = point_getz(point);
		}
		/*if (irandom(1000)<1 && state->comp_count[c_light]<256)
		{
			v3 lp;
			float radius;
			lp.x = x*BLOCK_SIZE+random(1024)-512;
			lp.y = y*BLOCK_SIZE+random(1024)-512;
			lp.z = z*BLOCK_SIZE+random(1024)-512;
			radius = 8;
			light_create(state,lp,radius);
		}*/
	}
	// fatten blocks
	int b,w;
	for(w = 0; w<2; w++)
	{
		load_screen_draw(15);
		blocks = state->block_count;
		for(i = 0; i<blocks;i++)
		{
			b = state->block_list[i];
			x = point_getx(b);
			y = point_gety(b);
			z = point_getz(b);

			
			if (!block_at_bounded(state,x-1,y,z))
				block_create(state,x-1,y,z);
			if (!block_at_bounded(state,x,y+1,z))
				block_create(state,x,y+1,z);
			if (!block_at_bounded(state,x,y,z-1))
				block_create(state,x,y,z-1);
		}
	}
	
	for(w=0; w<2; w++)
	{
		load_screen_draw(15);
		blocks = state->block_count;
		for(i=0; i<blocks; i++)
		{
			b = state->block_list[i];
			x = point_getx(b);
			y = point_gety(b);
			z = point_getz(b);

			if (!block_at_bounded(state,x,y+1,z) && block_at_bounded(state,x+1,y+1,z))
				block_create(state,x,y+1,z);
		}
	}

	// find suitable collision-free place to spawn player
	while(block_at(state,xstart,ystart,zstart) || block_at(state,xstart,ystart,zstart+1))
		zstart++;

	// spawn player
	v3 spawn;
	spawn.x = xstart * BLOCK_SIZE + 16.0;
	spawn.y = ystart * BLOCK_SIZE + 16.0;
	spawn.z = zstart * BLOCK_SIZE + 16.0;
	
	player_create(state,spawn);
	printf("level block count: %d\n",state->block_count);
}

void level_next(game_state *state, int clearModels)
{
	if (clearModels)
	{
		// destroy grass/level model
		level_model_destroy(state);
		grass_model_destroy(state);
		model_destroy(state->dust_model);
	}
	// clear old game_states memory
	p_state p = state->pstate;
	game_state_clear(state);
	state->pstate = p;

	// generate new level data
	level_gen(state);
	// build new level model
	load_screen_draw(10);
	level_model_build(state);
	load_screen_draw(10);	
	grass_model_build(state);
	load_screen_draw(10);
	state->dust_model = dust_model_build(state);
}

// 10 bits for the x,y,z position of each point
int point_getx(int block)
{
	return block & 1023;
}

int point_gety(int block)
{
	return (block >> 10) & 1023;
}

int point_getz(int block)
{
	return (block >> 20) & 1023;
}

int point_create(int x, int y, int z)
{
	return ((z<<20) | (y<<10) | x);
}

int block_at(game_state *state,int x, int y, int z)
{
	return (x<LEVEL_SIZE && x>=0 && y<LEVEL_SIZE && y>=0 && z<LEVEL_SIZE && z>=0 && bit_get(state,x,y,z));
}

int block_at_bounded(game_state *state, int x, int y, int z)
{
	return (x>=LEVEL_SIZE || x<0 || y>=LEVEL_SIZE || y<0 || z>=LEVEL_SIZE || z<0 || bit_get(state,x,y,z));
}

void block_create(game_state *state, int x, int y, int z)
{
	if (state->block_count >= MAX_BLOCKS || x>=LEVEL_SIZE || x<0 || y>=LEVEL_SIZE || y<0 || z>=LEVEL_SIZE || z<0)
		return;
	//state->block_grid[x][y][z] = 1;//state->block_count;
	bit_set(state,x,y,z);
	state->block_list[state->block_count] = point_create(x,y,z);
	state->block_count++;
}


void delete_queue_enter(game_state *state, int point)
{
	state->delete_queue[state->dq_end] = point;
	state->dq_end = (state->dq_end+1) % DELETE_QUEUE_SIZE;
}
int delete_queue_remove(game_state *state)
{
	int result = state->delete_queue[state->dq_front];
	state->dq_front = (state->dq_front+1) % DELETE_QUEUE_SIZE;
	return result;
}
int delete_queue_is_empty(game_state *state)
{
	return (state->dq_front == state->dq_end);
}