#include <SDL2/SDL.h>
#include "state.h"
#include "gmath.h"

void game_simulate(game_state *state, const Uint8 *key_state)
{
	float *x,*y,dir;
	x = &(state->camx);
	y = &(state->camy);
	dir = state->camdir;
		if (key_state[SDL_SCANCODE_W])
			move_to(x,y,dir,1.f);
		if (key_state[SDL_SCANCODE_S])
			move_to(x,y,dir+180.f,1.f);
		if (key_state[SDL_SCANCODE_D])
			move_to(x,y,dir-90.f,1.f);
		if (key_state[SDL_SCANCODE_A])
			move_to(x,y,dir+90.f,1.f);
		if (key_state[SDL_SCANCODE_Q])
			state->camz+=1.f;
		if (key_state[SDL_SCANCODE_E])
			state->camz+=-1.f;

		if (key_state[SDL_SCANCODE_LEFT])
			state->camdir+=1.;
		if (key_state[SDL_SCANCODE_RIGHT])
			state->camdir-=1.;
		if (key_state[SDL_SCANCODE_UP])
			state->camzdir+=1.;
		if (key_state[SDL_SCANCODE_DOWN])
			state->camzdir-=1.;

		if (state->camzdir>89.f)
			state->camzdir = 89.f;
		if (state->camzdir<-89.f)
			state->camzdir = -89.f;
}