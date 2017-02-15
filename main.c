#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

int main()
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *window = SDL_CreateWindow(
		"Nebula",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640,360,
		SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP
	);

	SDL_Renderer *renderer = SDL_CreateRenderer(window,-1,0);
	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"nearest");
	//SDL_RenderSetLogicalSize(renderer,640,360);

	SDL_GLContext glContext;
	glContext = SDL_GL_CreateContext(window);
	

	SDL_Event event;
	int quit = 0;

	while(!quit)
	{
		while(SDL_PollEvent(&event))
		{
			if (event.type==SDL_QUIT)
				quit=1;
			if (event.type == SDL_KEYDOWN)
				if (event.key.keysym.sym == SDLK_ESCAPE)
					quit = 1;
		}
		glClearColor(0.50f,0.125f,0.25f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		SDL_GL_SwapWindow(window);
		SDL_Delay(16);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
