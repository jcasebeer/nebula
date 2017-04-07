#include <stdio.h>
#include <stdlib.h>
#include "render.h"
#include "state.h"
#include "gmath.h"

void level_gen(game_state *state)
{
	int blocks = LEVEL_SIZE * BLOCK_SIZE * 3;
	int x,y,z;
	int i;

	// set all initial values in block_grid to -1
	int *gptr = &(state->block_grid[0][0][0]);
	for(i=0;i<LEVEL_SIZE*LEVEL_SIZE*LEVEL_SIZE;i++)
	{
		*(gptr+i) = -1;
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
		if (state->block_grid[x][y][z] == -1)
		{
			block_create(state,x,y,z);
		}

		x+=choose3(1,0,-1);
		y+=choose3(1,0,-1);
		z+=choose3(1,0,-1);
		while (x>=LEVEL_SIZE || x < 0)
			x = irandom(LEVEL_SIZE);
		while (y>=LEVEL_SIZE || y < 0)
			y = irandom(LEVEL_SIZE);
		while (z>=LEVEL_SIZE || z < 0)
			z = irandom(LEVEL_SIZE);

		blocks--;
	}
	// fatten blocks
	int b;
	for(int w = 0; w<2; w++)
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

void level_next(game_state *state)
{
	// destory level model
	model_destroy(state->level_model);
	model_destroy(state->grass_model);
	// clear old game_states memory
	game_state_clear(state);
	// generate new level data
	level_gen(state);
	// build new level model
	state->level_model = level_model_build(state);	
	state->grass_model = grass_model_build(state);
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
	return (x<LEVEL_SIZE && x>=0 && y<LEVEL_SIZE && y>=0 && z<LEVEL_SIZE && z>=0 && state->block_grid[x][y][z]>-1);
}

int block_at_bounded(game_state *state, int x, int y, int z)
{
	return (x>=LEVEL_SIZE || x<0 || y>=LEVEL_SIZE || y<0 || z>=LEVEL_SIZE || z<0 || state->block_grid[x][y][z]>-1);
}

void block_create(game_state *state, int x, int y, int z)
{
	if (state->block_count > MAX_BLOCKS)
		return;
	state->block_grid[x][y][z] = state->block_count;
	state->block_list[state->block_count] = point_create(x,y,z);
	state->block_count++;
}
