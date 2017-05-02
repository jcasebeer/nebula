#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include "gmath.h"
#include "render.h"
#include "state.h"
#include "sound.h"

int main(int argc, char *argv[])
{
	// hide unused parameter warning
	(void) argc;
	(void) argv;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	int width = 960;
	int height = 720;
	int fullscreen = 0;

	// turn on double buffering and set opengl version
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL,1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

	// create our window
	SDL_Window *window = SDL_CreateWindow(
		"Nebula",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width,height,
		SDL_WINDOW_OPENGL
	);

	// process priority
	//if (SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH)<0)
	//	printf("Thread Priority Error: %s\n",SDL_GetError());

	// grab the mouse
	SDL_SetWindowGrab(window,1);
	SDL_SetRelativeMouseMode(1);

	// create opengl context
	SDL_GL_CreateContext(window);
	// turn on vsync
	//SDL_GL_SetSwapInterval(1);

	// get our extensions
	glewInit();
	glDisable(GL_DITHER);

	// initialze some variables for the main loop
	#define TARGET_FPS 60
	int quit = 0;
	unsigned long int time = 0;
	unsigned long int timespent = 0;
	unsigned long int second_counter = 0;
	unsigned long int timer_res = 1000;
	unsigned long int sleeptime = timer_res/TARGET_FPS;
	int skip_max = 1;

	// sdl variables for handling input
	SDL_Event event;
	const Uint8 *key_state = SDL_GetKeyboardState(NULL);

	// generate our drawing surface
	surface_data *surf = surface_data_create(width,height,1.8);

	// init audio and create our audio object
	sound_data *sounds = sound_data_create();
	sounds->jump = sound_load("snd/jump.wav");
	sounds->rifle = sound_load("snd/rifle.wav");
	sounds->grapple_shoot = sound_load("snd/grapple_shoot.wav");
	sounds->grapple_stick = sound_load("snd/stick.wav");
	sounds->grapple_buzz = sound_load("snd/grappling.wav");
	sounds->grapple_end = sound_load("snd/grapple_end.wav");

	// create and load our texture data
	texture_data *textures = texture_data_create();
	textures->sprites = texture_load("tex/sprites.png",1024,1024);
	textures->shadow = texture_load("tex/shadow.png",256,256);

	// create our game_state
	game_state *state = game_state_create(sounds);
	p_state *pstate = p_state_create();
	 // seed rng
    time_seed_rng();
    pstate->weapons[0] = gen_gun();
	pstate->weapons[1] = gen_gun();

	// generate a level and build its model
	//level_gen(state);
	//state->level_model = level_model_build(state);
	level_next(state,0,pstate);

	int error = 1;
	int frames = 0;

	// main game loop
	while(!quit)
	{
		time = SDL_GetPerformanceCounter()*timer_res/SDL_GetPerformanceFrequency();
	
		while(SDL_PollEvent(&event))
		{
			if (event.type==SDL_QUIT)
					quit=1;
				if (event.type == SDL_KEYDOWN)
				{
					if (event.key.keysym.sym == SDLK_ESCAPE)
						quit = 1;
					
				}
				if (event.type == SDL_KEYUP)
				{
					if (event.key.keysym.sym == SDLK_RETURN)
					{
						state->next_level = 1;
					}
					if (event.key.keysym.sym == SDLK_F5)
					{
						if (skip_max == 1)
							skip_max = 2;
						else
							skip_max = 1;

						sleeptime = timer_res/(TARGET_FPS/skip_max);
					}
					if (event.key.keysym.sym == SDLK_F4)
					{
						if (fullscreen)
						{
							fullscreen = 0;
							SDL_SetWindowFullscreen(window,0);
						}
						else
						{
							fullscreen = 1;
							SDL_SetWindowFullscreen(window,SDL_WINDOW_FULLSCREEN_DESKTOP);
						}
					}

					if (event.key.keysym.sym == SDLK_F6)
					{
						if (surf->gamma < 2.6)
							surf->gamma += 0.2;
						else
							surf->gamma = 1.6;
					}
				}
		}

		for (int i = 0; i<skip_max; i++)
		{
			SDL_PumpEvents();
			game_simulate(state,key_state);
		}

		game_render_pp(state,window,textures,surf);
		SDL_GL_SwapWindow(window);
		frames++;

		if (state->next_level)
		{
			level_next(state,1,pstate);
			state->next_level = 0;
		}

		// opengl error reporting
		while (error != GL_NO_ERROR)
		{
			error = glGetError();
			switch(error)
			{
				case GL_NO_ERROR:
				printf("No Error!\n");
				break;

				case GL_INVALID_ENUM:
				printf("GL_INVALID_ENUM\n");
				break;

				case GL_INVALID_VALUE:
				printf("GL_INVALID_VALUE\n");
				break;

				case GL_INVALID_OPERATION:
				printf("GL_INVALID_OPERATION\n");
				break;

				case GL_STACK_OVERFLOW:
				printf("GL_STACK_OVERFLOW\n");
				break;

				case GL_STACK_UNDERFLOW:
				printf("GL_STACK_UNDERFLOW\n");
				break;

				case GL_OUT_OF_MEMORY:
				printf("GL_OUT_OF_MEMORY\n");
				break;

				case GL_TABLE_TOO_LARGE:
				printf("GL_TABLE_TOO_LARGE\n");
				break;

			}
		}

		if (time - second_counter > timer_res)
		{
			state->frame_time = (int)frames/((time-second_counter)/timer_res);
			frames = 0;
			second_counter = SDL_GetPerformanceCounter()*timer_res/SDL_GetPerformanceFrequency();
		}
			
		// limit framerate
		timespent = SDL_GetPerformanceCounter()*timer_res/SDL_GetPerformanceFrequency() - time;
		while (timespent<sleeptime)
		{
			timespent = SDL_GetPerformanceCounter()*timer_res/SDL_GetPerformanceFrequency() - time;
		}
	}
	free(pstate);
	model_destroy(state->level_model);
	model_destroy(state->grass_model);
	game_state_destroy(state);
	texture_destroy(textures->sprites);
	sound_free(sounds->jump); 
	sound_free(sounds->rifle); 
	sound_free(sounds->grapple_shoot);
	sound_free(sounds->grapple_stick); 
	sound_free(sounds->grapple_buzz);
	sound_free(sounds->grapple_end);
	sound_data_destroy(sounds);
	free(textures);
	surface_data_destroy(surf);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
