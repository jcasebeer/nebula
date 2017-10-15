#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <string.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include "gmath.h"
#include "render.h"
#include "state.h"
#include "sound.h"

sound_data *SOUND;
SDL_Window *window;
load_state LOAD_STATE;

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
	window = SDL_CreateWindow(
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
	printf("OpenGL Version: %s\n",glGetString(GL_VERSION));
	// turn off vsync
	SDL_GL_SetSwapInterval(0);

	// get our extensions
	#ifndef NO_SHADER
	glewInit();
	#endif
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
	int key_state_size;
	const Uint8 *key_state = SDL_GetKeyboardState(&key_state_size);
	Uint8 *prev_key_state = malloc(sizeof(Uint8)*key_state_size);

	#ifndef NO_SHADER
	// generate our drawing surface
	surface_data *surf = surface_data_create(width,height,1.8);
	#endif

	// init audio and create our audio object
	SOUND = sound_data_create();
	SOUND->jump = sound_load("snd/jump.wav");
	SOUND->rifle = sound_load("snd/rifle.wav");
	SOUND->grapple_shoot = sound_load("snd/grapple_shoot.wav");
	SOUND->grapple_stick = sound_load("snd/stick.wav");
	SOUND->grapple_buzz = sound_load("snd/grappling.wav");
	SOUND->grapple_end = sound_load("snd/grapple_end.wav");
	SOUND->reload = sound_load("snd/reload.wav");

	// create and load our texture data
	texture_data *textures = texture_data_create();
	textures->sprites = texture_load("tex/sprites.png",1024,1024);
	textures->shadow = texture_load("tex/shadow.png",64,64);

	// create our game_state
	game_state *state = game_state_create(SOUND);
	//p_state *pstate = p_state_create();
	// seed rng
	seed_rng(0);
    //time_seed_rng();

    state->pstate.weapons[0] = gen_gun();
	state->pstate.weapons[1] = gen_gun();

	// generate a level and build its model
	//level_gen(state);
	//state->level_model = level_model_build(state);
	level_next(state,0);

	// turn on vsync
	SDL_GL_SetSwapInterval(1);

	int error = 1;
	int frames = 0;

	// main game loop
	while(!quit)
	{
		time = SDL_GetPerformanceCounter()*timer_res/SDL_GetPerformanceFrequency();
		memcpy(prev_key_state,key_state,sizeof(Uint8)*key_state_size);

		while(SDL_PollEvent(&event))
		{
			if (event.type==SDL_QUIT)
					quit=1;
		}

		if (key_pressed(SDL_SCANCODE_ESCAPE))
		{
			quit = 1;
		}

		if (key_pressed(SDL_SCANCODE_F4))
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

		if (key_pressed(SDL_SCANCODE_RETURN) && state->next_level == 0)
		{
			state->next_level = 1;
		}

		if (key_pressed(SDL_SCANCODE_F5))
		{
			if (skip_max == 1)
				skip_max = 2;
			else
				skip_max = 1;

			sleeptime = timer_res/(TARGET_FPS/skip_max);
		}

		#ifndef NO_SHADER
		if (key_pressed(SDL_SCANCODE_F6))
		{
			if (surf->gamma < 2.6)
				surf->gamma += 0.2;
			else
				surf->gamma = 1.6;
		}

		if (key_pressed(SDL_SCANCODE_F7))
		{
			surf->lines = !(surf->lines);
		}
		#endif

		for (int i = 0; i<skip_max; i++)
		{
			if (i==1)
				memcpy(prev_key_state,key_state,sizeof(Uint8)*key_state_size);
			game_simulate(state,key_state,prev_key_state);
		}

		#ifdef NO_SHADER
		game_render(state,window,textures);
		#else
		game_render_pp(state,window,textures,surf);
		#endif

		draw_hud(state,window,textures);
		SDL_GL_SwapWindow(window);
		frames++;

		if (state->next_level)
		{
			SDL_GL_SetSwapInterval(0);
			memset(&LOAD_STATE,0,sizeof(LOAD_STATE));
			level_next(state,1);
			state->next_level = 0;
			SDL_GL_SetSwapInterval(1);
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
	//free(pstate);
	level_model_destroy(state);
	grass_model_destroy(state);
	game_state_destroy(state);
	texture_destroy(textures->sprites);
	sound_free(SOUND,SOUND->jump); 
	sound_free(SOUND,SOUND->rifle); 
	sound_free(SOUND,SOUND->grapple_shoot);
	sound_free(SOUND,SOUND->grapple_stick); 
	sound_free(SOUND,SOUND->grapple_buzz);
	sound_free(SOUND,SOUND->grapple_end);
	sound_free(SOUND,SOUND->reload);
	sound_data_destroy(SOUND);
	free(textures);
	free(prev_key_state);
	#ifndef NO_SHADER
	surface_data_destroy(surf);
	#endif
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
