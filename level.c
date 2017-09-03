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
	float red,green,blue,csum;
	red = 1.f+random(10.f);
	green = 1.f+random(10.f);
	blue = 1.f+random(10.f);
	csum = red+green+blue;
	red/=csum;
	green/=csum;
	blue/=csum;
	state->levelColor[0] = red;//colors[color].x/255.f;
	state->levelColor[1] = green;//colors[color].y/255.f;
	state->levelColor[2] = blue;//colors[color].z/255.f;
	state->levelColor[3] = 1.f;
	int blocks = LEVEL_SIZE * BLOCK_SIZE * 3;
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
	x = LEVEL_SIZE / 2;
	y = x;
	z = x;

	int xstart, ystart, zstart;
	xstart = x;
	ystart = y;
	zstart = z; 

	while(blocks>0)
	{
		if (!bit_get(state,x,y,z))
		{
			block_create(state,x,y,z);
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
		blocks--;
	}
	// fatten blocks
	int b,w;
	for(w = 0; w<2; w++)
	{
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
		blocks = state->block_count;
		for(i=0; i<blocks; i++)
		{
			b = state->block_list[i];
			x = point_getx(b);
			y = point_gety(b);
			z = point_getz(b);

			if (!block_at_bounded(state,x,y+1,z) && block_at_bounded(state,x+1,y+1,z))
				block_create(state,x,y+1,z);
			
			//if (!block_at_bounded(state,x,y,z+1) && block_at_bounded(state,x,y,z+2))
			//	block_create(state,x,y,z+1);

			//if (!block_at_bounded(state,x,y+1,z) && block_at_bounded(state,x,y+2,z))
			//	block_create(state,x,y+1,z);

			//if (!block_at_bounded(state,x+1,y,z) && block_at_bounded(state,x+2,y,z))
			//	block_create(state,x+1,y,z);

			//if (!block_at_bounded(state,x,y+1,z) && block_at_bounded(state,x-1,y+1,z))
			//	block_create(state,x-1,y+1,z);

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
	level_model_build(state);	
	grass_model_build(state);
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

