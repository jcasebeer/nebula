#include <SDL2/SDL.h>
#include "state.h"

void game_simulate(game_state *state, const Uint8 *key_state)
{
	if (key_state[SDL_SCANCODE_D])
			state->camx-=1.;
		if (key_state[SDL_SCANCODE_A])
			state->camx+=1.;

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
	return;
}