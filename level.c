#include <stdio.h>
#include <stdlib.h>
#include "state.h"
#include "gmath.h"

void level_gen(game_state *state)
{
	int blocks = LEVEL_SIZE * BLOCK_SIZE;
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

	while(blocks>0)
	{
		x+=choose3(1,0,-1);
		y+=choose3(1,0,-1);
		z+=choose3(1,0,-1);
		if (x>LEVEL_SIZE || x < 0)
			x = irandom(LEVEL_SIZE);
		if (y>LEVEL_SIZE || y < 0)
			y = irandom(LEVEL_SIZE);
		if (z>LEVEL_SIZE || z < 0)
			z = irandom(LEVEL_SIZE);

		if (state->block_grid[x][y][z] == -1)
		{
			block_create(state,x,y,z);
		}

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

			
			if (!block_at_bounded(state,x+1,y,z))
				block_create(state,x+1,y,z);
			if (!block_at_bounded(state,x,y+1,z))
				block_create(state,x,y+1,z);
			if (!block_at_bounded(state,x,y,z+1))
				block_create(state,x,y,z+1);
		}
	}
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
	if (x>=LEVEL_SIZE || x<0 || y>=LEVEL_SIZE || y<0 || z>=LEVEL_SIZE || z<0)
		return 1;
	else
		return state->block_grid[x][y][z]>-1;
}

void block_create(game_state *state, int x, int y, int z)
{
	state->block_grid[x][y][z] = state->block_count;
	state->block_list[state->block_count] = point_create(x,y,z);
	state->block_count++;
}