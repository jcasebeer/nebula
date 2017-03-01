#include "state.h"
#include "gmath.h"

void level_gen(game_state *state)
{
	int blocks = LEVEL_SIZE * BLOCK_SIZE * 3;
	int x,y,z;
	state->block_count = 1;
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

		if (state->block_grid[x][y][z] == 0)
		{
			state->block_grid[x][y][z] = state->block_count;
			state->block_list[state->block_count] = point_create(x,y,z);
			state->block_count++;
		}

		blocks--;
	}
	/*
	blocks = state->block_count;
	for(int i = 1; i<blocks;i++)
	{

	}*/
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