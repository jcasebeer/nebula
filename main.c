#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <math.h>
#include <GL/glu.h>

void normalize(float *vec)
{
	float m = 1.0/sqrtf(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
	vec[0]/=m;
	vec[1]/=m;
	vec[2]/=m;
}

void cross(float *where, float *a, float *b)
{
	where[0] = a[1]*b[2] - a[2]*b[1];
	where[1] = a[2]*b[0] - a[0]*b[2];
	where[2] = a[0]*b[1] - a[1]*b[0];
}

// not working, falling back to glu for now
void draw_position_camera(float x, float y, float z, float xto, float yto, float zto)
{
	float m[4][4] = {
		{1,0,0,0},
		{0,1,0,0},
		{0,0,1,0},
		{0,0,0,1}
	};

	glGetFloatv(GL_MODELVIEW_MATRIX,(GLfloat *)m);
	float up[3], right[3], forward[3];

	forward[0] = xto - x;
	forward[1] = yto - y;
	forward[2] = zto - z;

	up[0] = 0.;
	up[1] = 0.;
	up[2] = 1.;

	normalize(forward);
	cross(right,forward,up);
	normalize(right);

	cross(up,right,forward);

	m[0][0] = right[0];
	m[1][0] = right[1];
	m[2][0] = right[2];

	m[0][1] = up[0];
	m[1][1] = up[1];
	m[2][1] = up[2];

	m[0][2] = -forward[0];
	m[1][2] = -forward[1];
	m[2][2] = -forward[2];

	//glMultMatrixf(&m[0][0]);
	glLoadMatrixf(&m[0][0]);
	glTranslated(-x,-y,-z);
}

float lengthdir_x(float len, float dir)
{
	const float DEG2RAD = 3.14159265/180;
	return len*cos(DEG2RAD*dir);
}

float lengthdir_y(float len, float dir)
{
	const float DEG2RAD = 3.14159265/180;
	return len*sin(DEG2RAD*dir);
}

void draw_set_frustum(float fov, float ar,float znear, float zfar)
{
	const float DEG2RAD = 3.14159265/180;

	float t = tan(fov/2 * DEG2RAD);
	float height = znear * t;
	float width = height * ar;

	glFrustum(-width,width,-height,height,znear,zfar);
}

int main()
{
	SDL_Window *window = SDL_CreateWindow(
		"Nebula",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640,360,
		SDL_WINDOW_OPENGL
	);

	SDL_Renderer *renderer = SDL_CreateRenderer(window,-1,0);
	SDL_RenderSetLogicalSize(renderer,640,360);
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2);

	SDL_GL_CreateContext(window);

	SDL_Event event;
	int quit = 0;
	unsigned int time = 0;
	unsigned int timespent = 0;
	#define TARGET_FPS 60
	unsigned int sleeptime = 1000/TARGET_FPS;
	const Uint8 *key_state = SDL_GetKeyboardState(NULL);

	// temporary cam vars
	float x = 0.;
	float y = 0.;
	float z = 0.;

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
			dir+=4.;
		if (key_state[SDL_SCANCODE_RIGHT])
			dir-=4.;
		if (key_state[SDL_SCANCODE_UP])
			zdir+=4.;
		if (key_state[SDL_SCANCODE_DOWN])
			zdir-=4.;

		if (zdir>89.9)
			zdir = 89.9;
		if (zdir<-89.9)
			zdir = -89.9;

		glClearColor(0.50f,0.125f,0.25f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		draw_set_frustum(120.,16./9.,1.,32000.);
		//draw_set_frustum(90,16/9,1,32000);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(
			x,
			y,
			z+8,
			x+lengthdir_x(lengthdir_x(1,zdir),dir),
            y+lengthdir_y(lengthdir_x(1,zdir),dir),
            z+8+lengthdir_y(1,zdir),
            0.,0.,1.
		);
		/*draw_position_camera(
			x,
            y,
            z+8,
            x+lengthdir_x(lengthdir_x(1,zdir),dir),
            y+lengthdir_y(lengthdir_y(1,zdir),dir),
            z+8+lengthdir_y(1,zdir)
		);*/

		glBegin(GL_TRIANGLE_FAN);
		glColor3f(1.f,0.f,0.f);
		glVertex3f(-100.f,-100.f,0.f);
		glColor3f(0.f,1.f,0.f);
		glVertex3f(100.f,-100.f,0.f);
		glColor3f(0.f,0.f,1.f);
		glVertex3f(100.f,100.f,0.f);
		glColor3f(1.f,1.f,1.f);
		glVertex3f(-100.f,100.f,0.f);
		glEnd();

		SDL_GL_SwapWindow(window);
		
		timespent = SDL_GetTicks() - time;
		if (timespent<sleeptime)
			SDL_Delay(sleeptime - timespent);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
