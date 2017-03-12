#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include "gmath.h"
#include "render.h"
#include "state.h"

int main()
{
	int width = 1280;
	int height = 720;

	// create our window
	SDL_Window *window = SDL_CreateWindow(
		"Nebula",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width,height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);

	// turn on double buffering and vsync
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
	SDL_GL_SetSwapInterval(1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0);

	// create opengl context
	SDL_GL_CreateContext(window);

	// get our extensions
	glewInit();

	// initialze some variables for the main loop
	#define TARGET_FPS 60
	int quit = 0;
	unsigned int time = 0;
	unsigned int timespent = 0;
	unsigned int sleeptime = 1000/TARGET_FPS;

	// sdl variables for handling input
	SDL_Event event;
	const Uint8 *key_state = SDL_GetKeyboardState(NULL);

	// generate our drawing surface
	surface_data *surf = surface_data_create(width,height);

	// create and load our texture data
	texture_data *textures = texture_data_create();
	textures->sprites = texture_load("tex/face2.png",1024,1024);
	textures->shadow = texture_load("tex/shadow.png",256,256);

	// create our game_state
	game_state *state = game_state_create();

    // seed rng
    time_seed_rng();

	// generate a level and build its model
	level_gen(state);
	state->level_model = level_model_build(state);

	// move camera to center of level (for testing)
	state->camx = LEVEL_SIZE*BLOCK_SIZE/2.;
	state->camy = state->camx;
	state->camz = state->camx;

	// main game loop
	while(!quit)
	{
		time = SDL_GetTicks();

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
				}
		}

		game_simulate(state,key_state);
		game_render_pp(state,window,textures,surf);

		if (state->next_level)
		{
			level_next(state);
			state->next_level = 0;
		}

		SDL_GL_SwapWindow(window);
		
		timespent = SDL_GetTicks() - time;
		if (timespent<sleeptime)
			SDL_Delay(sleeptime - timespent);

	}
	model_destroy(state->level_model);
	game_state_destroy(state);
	texture_destroy(textures->sprites);
	free(textures);
	surface_data_destroy(surf);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

/*int object_create(game_state *state, float x, float y,float z)
{
	int obj = entity_create(state);

	entity_component_add(state,obj,c_position);
	v3 *pos = &(state->position[obj]);
	pos->x = x;
	pos->y = y;
	pos->z = z;

	return obj;
}

int object_create_2(game_state *state)
{
	int obj = entity_create(state);
	entity_component_add(state,obj,c_velocity);
	return obj;
}



int main()
{
	time_seed_rng();
	game_state *state = game_state_create();
	int width = 1280;
	int height = 720;

	SDL_Window *window = SDL_CreateWindow(
		"Nebula",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width,height,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE 
		//| SDL_WINDOW_FULLSCREEN_DESKTOP
	);

	SDL_GL_CreateContext(window);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
	SDL_GL_SetSwapInterval(1);

	GLuint texture = texture_load("tex/face.png",1024,1024);
	GLuint texture2 = texture_load("tex/face2.png",1024,1024);

	// test level model building
	level_gen(state);
	GLuint level_model = level_model_build(state);

	SDL_Event event;
	int quit = 0;
	unsigned int time = 0;
	unsigned int timespent = 0;
	#define TARGET_FPS 60
	unsigned int sleeptime = 1000/TARGET_FPS;
	const Uint8 *key_state = SDL_GetKeyboardState(NULL);

	// temporary cam vars
	float x = LEVEL_SIZE*BLOCK_SIZE/2.;
	float y = x;
	float z = x;

	float tx = x;
	float ty = y;
	float tz = z-10;

	float dir = 0.;
	float zdir = 0.;


	while(!quit)
	{
		time = SDL_GetTicks();

		while(SDL_PollEvent(&event))
		{
			if (event.type==SDL_QUIT)
				quit=1;
			if (event.type == SDL_KEYDOWN)
				if (event.key.keysym.sym == SDLK_ESCAPE)
					quit = 1;
		}

		
		if (key_state[SDL_SCANCODE_D])
			x-=1.;
		if (key_state[SDL_SCANCODE_A])
			x+=1.;

		if (key_state[SDL_SCANCODE_LEFT])
			dir+=1.;
		if (key_state[SDL_SCANCODE_RIGHT])
			dir-=1.;
		if (key_state[SDL_SCANCODE_UP])
			zdir+=1.;
		if (key_state[SDL_SCANCODE_DOWN])
			zdir-=1.;

		if (zdir>89.f)
			zdir = 89.f;
		if (zdir<-89.f)
			zdir = -89.f;

		glClearColor(0.50f,0.125f,0.25f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		SDL_GetWindowSize(window, &width, &height);
		glViewport(0,0,width,height);
		draw_set_frustum(90.,(float)width/height,1.,32000.);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		draw_position_camera(
			x,
            y,
            z+8,
            x+lengthdir_x(lengthdir_x(1,zdir),dir),
            y+lengthdir_y(lengthdir_x(1,zdir),dir),
            z+8+lengthdir_y(1,zdir)
		);

		glPointSize(2.0);
		model_draw(level_model);


		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER,0.f);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,texture);
		glPushMatrix();
		glTranslatef(tx,ty,tz);
		glBegin(GL_TRIANGLE_FAN);

		glTexCoord2f(0.f,0.f);
		//glColor3f(1,0,0);
    	glVertex3f(-8,+8,0);

    	glTexCoord2f(1.f,0.f);
    	//glColor3f(0,1,0);
    	glVertex3f(+8,+8,0);

    	glTexCoord2f(1.f,1.f);
    	//glColor3f(0,0,1);
    	glVertex3f(+8,-8,0);


    	glTexCoord2f(0.f,1.f);
    	//glColor3f(1,1,1);
    	glVertex3f(-8,-8,0);
		glEnd();
		glPopMatrix();

		glBindTexture(GL_TEXTURE_2D,texture2);
		glPushMatrix();
		glTranslatef(tx-100,ty,tz);
		glBegin(GL_TRIANGLE_FAN);

		glTexCoord2f(0.f,0.f);
		//glColor3f(1,0,0);
    	glVertex3f(-8,+8,0);

    	glTexCoord2f(1.f,0.f);
    	//glColor3f(0,1,0);
    	glVertex3f(+8,+8,0);

    	glTexCoord2f(1.f,1.f);
    	//glColor3f(0,0,1);
    	glVertex3f(+8,-8,0);


    	glTexCoord2f(0.f,1.f);
    	//glColor3f(1,1,1);
    	glVertex3f(-8,-8,0);
		glEnd();
		glPopMatrix();

		glBindTexture(GL_TEXTURE_2D,0);

		SDL_GL_SwapWindow(window);
		
		timespent = SDL_GetTicks() - time;
		if (timespent<sleeptime)
			SDL_Delay(sleeptime - timespent);
	}

	texture_destroy(texture);
	texture_destroy(texture2);
	game_state_destroy(state);
	model_destroy(level_model);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}*/
