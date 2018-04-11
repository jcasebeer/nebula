#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>
#include "render.h"
#include "gmath.h"
#include "state.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "shaders.h"
#include "comps.h"

void draw_text(float xstart, float ystart, float size, const char *text);
void draw_text_br(float xstart, float ystart, float size, const char *text);
void draw_player_gun(game_state *state);
float tex_id_x(int id);
float tex_id_y(int id);

static void drawLightBalls(game_state *state);

#ifndef NO_SHADER
surface_data *surface_data_create(int width, int height, float gamma)
{
	surface_data *surf = malloc(sizeof(surface_data));
	surf->width = width;
	surf->height = height;
	surf->gamma = gamma;
	surf->lines = 1;

	// fbo texture
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1,&(surf->fbo_texture));
	glBindTexture(GL_TEXTURE_2D,surf->fbo_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
  	glBindTexture(GL_TEXTURE_2D,0);

  	// depth buffer
  	glGenRenderbuffers(1,&(surf->rbo_depth));
  	glBindRenderbuffer(GL_RENDERBUFFER,surf->rbo_depth);
  	glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT16,width,height);
  	glBindRenderbuffer(GL_RENDERBUFFER,0);

  	// fbo
  	glGenFramebuffers(1,&(surf->fbo));
  	glBindFramebuffer(GL_FRAMEBUFFER,surf->fbo);
  	// attach texture
  	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,surf->fbo_texture,0);
  	// attach depth
  	glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,surf->rbo_depth);
  	GLenum status;
  	if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
  	{
  		printf("glCheckFramebufferStatus: error");
  		return NULL;
  	}
  	glBindFramebuffer(GL_FRAMEBUFFER,0);

  	GLfloat fbo_vertices[] = {
  		-1,-1,
  		 1,-1,
  		-1, 1,
  		 1, 1
  	};

  	glGenBuffers(1,&(surf->vbo_fbo_verts));
  	glBindBuffer(GL_ARRAY_BUFFER,surf->vbo_fbo_verts);
  	glBufferData(GL_ARRAY_BUFFER,sizeof(fbo_vertices),fbo_vertices,GL_STATIC_DRAW);
  	glBindBuffer(GL_ARRAY_BUFFER,0);

  	GLint result;
  	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  	glShaderSource(VertexShaderID,1,&post_shader_v,NULL);
  	glCompileShader(VertexShaderID);
  	glGetShaderiv(VertexShaderID,GL_COMPILE_STATUS, &result);

  	printf("Vertex Shader Result: %d\n",(int)result);

 	glShaderSource(FragmentShaderID,1,&post_shader_f,NULL);
  	glCompileShader(FragmentShaderID);
  	glGetShaderiv(FragmentShaderID,GL_COMPILE_STATUS, &result);

  	printf("Fragment Shader Result: %d\n",(int)result);

  	surf->post_shader = glCreateProgram();
  	glAttachShader(surf->post_shader,VertexShaderID);
  	glAttachShader(surf->post_shader,FragmentShaderID);
  	glLinkProgram(surf->post_shader);

  	glGetProgramiv(surf->post_shader, GL_LINK_STATUS, &result);
  	printf("Shader link status: %d\n",(int)result);

  	glGetProgramiv(surf->post_shader, GL_VALIDATE_STATUS, &result);
  	printf("Shader validation: %d\n", (int)result);

  	surf->a_vcoord = glGetAttribLocation(surf->post_shader,"v_coord");
  	surf->u_fbo_texture = glGetUniformLocation(surf->post_shader,"fbo_texture");
  	surf->u_gamma = glGetUniformLocation(surf->post_shader,"gamma");
  	surf->u_lines = glGetUniformLocation(surf->post_shader,"lines");
  	surf->u_resx = glGetUniformLocation(surf->post_shader,"resx");
  	surf->u_resy = glGetUniformLocation(surf->post_shader,"resy");

  	glDetachShader(surf->post_shader,VertexShaderID);
  	glDetachShader(surf->post_shader,FragmentShaderID);

  	glDeleteShader(VertexShaderID);
  	glDeleteShader(FragmentShaderID);

  	return surf;
}

void surface_data_destroy(surface_data *surf)
{
	glDeleteRenderbuffers(1,&(surf->rbo_depth));
	glDeleteTextures(1,&(surf->fbo_texture));
	glDeleteFramebuffers(1,&(surf->fbo));
	glDeleteBuffers(1,&(surf->vbo_fbo_verts));
	glDeleteProgram(surf->post_shader);

	free(surf);
}

void game_render_pp(game_state *state, SDL_Window *window, texture_data *textures, surface_data *surf)
{
	int width, height;
	SDL_GetWindowSize(window, &width, &height);

	// resize fbo
	if (width != surf->width || height != surf->height)
	{
		surf->width = width;
		surf->height = height;
		glBindTexture(GL_TEXTURE_2D,surf->fbo_texture);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
		glBindTexture(GL_TEXTURE_2D,0);

		glBindRenderbuffer(GL_RENDERBUFFER,surf->rbo_depth);
		glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT16,width,height);
		glBindRenderbuffer(GL_RENDERBUFFER,0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER,surf->fbo);
	game_render(state,window,textures);
	glBindFramebuffer(GL_FRAMEBUFFER,0);

	glClearColor(0.f,0.f,0.f,1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_FRAMEBUFFER_SRGB);
	glUseProgram(surf->post_shader);
	//glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,surf->fbo_texture);
	glUniform1i(surf->u_fbo_texture,0);
	glUniform1i(surf->u_lines,surf->lines);
	glUniform1f(surf->u_gamma,surf->gamma);
	glUniform1f(surf->u_resx,surf->width);
	glUniform1f(surf->u_resy,surf->height);
	glEnableVertexAttribArray(surf->a_vcoord);

	glBindBuffer(GL_ARRAY_BUFFER,surf->vbo_fbo_verts);
	glVertexAttribPointer(
		surf->a_vcoord,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		0
	);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	glDisableVertexAttribArray(surf->a_vcoord);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindTexture(GL_TEXTURE_2D,0);
	glUseProgram(0);
	glDisable(GL_FRAMEBUFFER_SRGB);
	//glDisable(GL_TEXTURE_2D);
}

#endif

static void ProjectLights(game_state *state);
const GLuint LightID[7] = {
	GL_LIGHT1,
	GL_LIGHT2,
	GL_LIGHT3,
	GL_LIGHT4,
	GL_LIGHT5,
	GL_LIGHT6,
	GL_LIGHT7
};

const GLubyte Stipple0[128] = {
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55,
		0x55, 0x55, 0x55, 0x55
};
const GLubyte Stipple1[128] = {
		0x33, 0x33, 0x33, 0x33,
		0x33, 0x33, 0x33, 0x33,
		0xCC, 0xCC, 0xCC, 0xCC,
		0xCC, 0xCC, 0xCC, 0xCC,
		0x33, 0x33, 0x33, 0x33,
		0x33, 0x33, 0x33, 0x33,
		0xCC, 0xCC, 0xCC, 0xCC,
		0xCC, 0xCC, 0xCC, 0xCC,
		0x33, 0x33, 0x33, 0x33,
		0x33, 0x33, 0x33, 0x33,
		0xCC, 0xCC, 0xCC, 0xCC,
		0xCC, 0xCC, 0xCC, 0xCC,
		0x33, 0x33, 0x33, 0x33,
		0x33, 0x33, 0x33, 0x33,
		0xCC, 0xCC, 0xCC, 0xCC,
		0xCC, 0xCC, 0xCC, 0xCC,
		0x33, 0x33, 0x33, 0x33,
		0x33, 0x33, 0x33, 0x33,
		0xCC, 0xCC, 0xCC, 0xCC,
		0xCC, 0xCC, 0xCC, 0xCC,
		0x33, 0x33, 0x33, 0x33,
		0x33, 0x33, 0x33, 0x33,
		0xCC, 0xCC, 0xCC, 0xCC,
		0xCC, 0xCC, 0xCC, 0xCC,
		0x33, 0x33, 0x33, 0x33,
		0x33, 0x33, 0x33, 0x33,
		0xCC, 0xCC, 0xCC, 0xCC,
		0xCC, 0xCC, 0xCC, 0xCC,
		0x33, 0x33, 0x33, 0x33,
		0x33, 0x33, 0x33, 0x33,
		0xCC, 0xCC, 0xCC, 0xCC,
		0xCC, 0xCC, 0xCC, 0xCC 
};
void game_render(game_state *state, SDL_Window *window, texture_data *textures)
{
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	// clear background
	//glShadeModel(GL_FLAT);
	glPolygonStipple(Stipple0);
	GLfloat comp[4];
	GLfloat gcomp[4];
	GLfloat ldark[4];
	compliment(state->levelColor,comp);
	compliment(state->levelColor,gcomp);
	darken(comp,comp);
	darken(state->levelColor,ldark);

	// set up projection matrix and window size
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	int width, height;
	SDL_GetWindowSize(window, &width, &height);
	glViewport(0,0,width,height);
	draw_set_frustum(state->fov,(float)width/height,1.f,state->frust_length);

	// point camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	float dir_shake = state->camdir+(random(state->cam_shake) - state->cam_shake*0.5)/10.f;
	float zdir_shake = -clamp(state->camzdir+(random(state->cam_shake) - state->cam_shake*0.5)/10.f,-89,89);

	float xdir = lengthdir_x(lengthdir_x(1,zdir_shake),dir_shake);
	float ydir = lengthdir_y(lengthdir_x(1,zdir_shake),dir_shake);
	float zdir = lengthdir_y(1,zdir_shake);

	float xto = state->camx+xdir;
	float yto = state->camy+ydir;
	float zbob = state->camz+state->vheight+sin(state->view_bob)*2;
	float zto = zbob+zdir;
	
	// calculate difference between viewing angle and sun direction
	/*float sunx = 0.707107;
	float suny = 0.0;
	float sunz = 0.707107;

	float m = sqrtf(xdir*xdir + ydir*ydir + zdir*zdir);

	sunx -= xdir/m;
	suny -= ydir/m;
	sunz -= zdir/m;

	m = (sqrtf(sunx*sunx + suny*suny + sunz*sunz)/2.f)*0.4;

	glClearColor(m,m,m,1.f);*/
	glClearColor(0.f,0.f,0.f,1.f);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	draw_position_camera(
		state->camx,
        state->camy,
        zbob,
        xto,
        yto,
        zto
	);

	//draw_closest_block_pos(state);

	// draw level model
	glPointSize(1);
	//#ifdef NO_SHADER
	//	glShadeModel(GL_FLAT);
	//#else
	glShadeModel(GL_SMOOTH);
	//#endif
	glEnable(GL_CULL_FACE);
	glLineWidth(1);

	// light push
	glPushMatrix();
	
//	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,(float [4]){0.9f,0.9f,0.9f,1.f});
//	glLightfv(GL_LIGHT0, GL_AMBIENT, (float [4]){0.9f,0.9f,0.9f,1.f});
//	glLightfv(GL_LIGHT0, GL_DIFFUSE, state->levelColor);
//	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.f);
////	float polyLight = 512.f;
//	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 1.f/polyLight);
//	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION,0.f);

	//GLfloat light_pos[4];
	//light_pos[0] = state->camx;
	//light_pos[1] = state->camy;
	//light_pos[2] = state->camz;
	//light_pos[3] = 1.f;
//	glLightfv(GL_LIGHT0,GL_POSITION,light_pos);
//	glLightfv(GL_LIGHT0,GL_SPECULAR,(float [4]){0.f,0.f,0.f,0.f});

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_STIPPLE);
	glColor3f(1.0,0.7,0.7);
	drawSphere(state->camx + 64, state->camy, zbob + 64, 4, 12);
	glEnable(GL_DEPTH_TEST);
	glPolygonStipple(Stipple1);
	//drawLightBalls(state);

	glDisable(GL_POLYGON_STIPPLE);
	glPolygonStipple(Stipple0);
	//glEnable(GL_LIGHTING);

	//draw level model
	//glEnable(GL_LIGHT0);

//	ProjectLights(state);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures->shadow_1024);

	glFogf(GL_FOG_START,0.f);
	glFogf(GL_FOG_END,512.f);
	glFogi(GL_FOG_MODE,GL_LINEAR);
	glFogi(GL_FOG_COORD_SRC, GL_FRAGMENT_DEPTH);
	glFogfv(GL_FOG_COLOR,(float [4]){0.f,0.f,0.f,1.f}/*state->levelFogColor*/);
	glEnable(GL_FOG);

	level_model_draw(state);

	glBindTexture(GL_TEXTURE_2D,0);
//	glDisable(GL_TEXTURE_2D);
//	glDisable(GL_LIGHT0);
//	glDisable(GL_LIGHT1);
//	glDisable(GL_LIGHT2);
//	glDisable(GL_LIGHT3);
//	glDisable(GL_LIGHT4);
//	glDisable(GL_LIGHT5);
//	glDisable(GL_LIGHT6);
//	glDisable(GL_LIGHT7);
	//glDisable(GL_LIGHTING);
	// light pop
	//glPopMatrix();

	//glPushMatrix();

	glFogf(GL_FOG_START,0.f);
	glFogf(GL_FOG_END,256.f);
	glFogi(GL_FOG_MODE,GL_LINEAR);
	glFogi(GL_FOG_COORD_SRC, GL_FRAGMENT_DEPTH);
	glFogfv(GL_FOG_COLOR,state->levelFogColor);
	//glEnable(GL_FOG);
	
	glDisable(GL_CULL_FACE);
	
	glColor3f(state->levelGrassColor[0],state->levelGrassColor[1],state->levelGrassColor[2]);
	glEnable(GL_TEXTURE_2D);
	glAlphaFunc(GL_GREATER,0.f);
	glEnable(GL_ALPHA_TEST);
	glBindTexture(GL_TEXTURE_2D,textures->grass);
	glEnable(GL_POLYGON_STIPPLE);
	grass_model_draw(state);
	glDisable(GL_POLYGON_STIPPLE);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,0);
	glPushMatrix();
	glTranslatef(sin(state->dust_anim)*10,cos(state->dust_anim)*10,sin(state->dust_anim)*10);
	model_draw(state->dust_model);
	glPopMatrix();
	glColor3f(1.f,1.f,1.f);
	glEnable(GL_CULL_FACE);
	glDisable(GL_FOG);
	glPopMatrix();
	
	if (state->grapple != -1)
	{
		v3 *gpos = &(state->position[state->grapple]);
		//v3 *ppos = &(state->position[state->player]);
		glBegin(GL_LINE_STRIP);
		glVertex3f(xto,yto,zto-4.f);
		for(int i = 0; i<16; i++)
		{
			if (state->grapple_state == 0)
			{
				glVertex3f(
					lerp(xto,gpos->x,i/16.0)+random(i*2.0)-i,
					lerp(yto,gpos->y,i/16.0)+random(i*2.0)-i,
					lerp(zto-4.f,gpos->z,i/16.0)+random(i*2.0)-i-sin(i/16.f*3.14)*32
				);
			}
		}
		glVertex3f(gpos->x,gpos->y,gpos->z);
		glEnd();
	}

	glAlphaFunc(GL_GREATER,0.f);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_TEXTURE_2D);

	// test drawing textures
	glBindTexture(GL_TEXTURE_2D,textures->sprites);
	
	int *sprites = get_ec_set(state,c_sprite);
	for(int i = 0; iterate_ec_set(sprites,i); i++)
	{
		draw_sprite(state,sprites[i]);
	}

	draw_player_gun(state);

	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
	
	//glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}

void draw_hud(game_state *state, SDL_Window *window, texture_data *textures)
{
	int width, height;
	SDL_GetWindowSize(window, &width, &height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,width,height,0,-1,1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_ALPHA_TEST);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures->sprites);

	glClear(GL_DEPTH_BUFFER_BIT);
	/* ~~~~~~~~~ hud ~~~~~~~~~~~~~ */

	/*
		TODO:
			make string handling for debug and gui stuff way easier.
	*/
	char fps_string[16] = "\0";
	char red_string[4] = "\0";
	char green_string[4] = "\0";
	char blue_string[4] = "\0";
	itoa((int)(state->levelColor[0]*255.f),red_string);
	itoa((int)(state->levelColor[1]*255.f),green_string);
	itoa((int)(state->levelColor[2]*255.f),blue_string);
	itoa(state->frame_time,fps_string);
	draw_text(0,0,16,fps_string);
	draw_text(0,16,16,red_string);
	draw_text(0,32,16,green_string);
	draw_text(0,48,16,blue_string);
	//draw_text_br(width,height,16,"Wow! text in the bottom right corner!");

	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
}

texture_data *texture_data_create()
{
	//
	return malloc(sizeof(texture_data));
}

void draw_position_camera(float x, float y, float z, float xto, float yto, float zto)
{
	GLfloat m[4][4] = {
		{1.,0.,0.,0.},
		{0.,1.,0.,0.},
		{0.,0.,1.,0.},
		{0.,0.,0.,1.}
	};

	float forward[3],side[3],up[3];

	forward[0] = xto - x;
	forward[1] = yto - y;
	forward[2] = zto - z;

	up[0] = 0.;
	up[1] = 0.;
	up[2] = 1.;

	normalize(forward);
	// right = f x u
	cross(side,forward,up);
	normalize(side);

	// up = r x f
	cross(up,side,forward);

	m[0][0] = side[0];
	m[1][0] = side[1];
	m[2][0] = side[2];

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

void draw_set_frustum(float fov, float ar,float znear, float zfar)
{
	float t = tan(fov/2 * DEG2RAD);
	float height = znear * t;
	float width = height * ar;

	glFrustum(-width,width,-height,height,znear,zfar);
}

static int offset(int amount)
{
	//
	return irandom(amount) - (amount >> 1);
}

static void vertex(int x, int y, int z, float xnorm, float ynorm, float znorm, float uv_x, float uv_y, float norm_offs, int tex_id, float light)
{
	seed_rng(x*y-z);
	glColor3f(light,light,light);
	glTexCoord2f((uv_x+tex_id_x(tex_id))/16.f,(uv_y+tex_id_y(tex_id))/16.f);
	glNormal3f(xnorm,ynorm,znorm);
	glVertex3i(x+offset(6),y+offset(6),z+offset(6));
}

static void nvertex(float x, float y, float z, float xnorm, float ynorm, float znorm)
{
	seed_rng(point_create(x,y,z));
	float NORMAL_OFFSET = random(10.f);
	xnorm += random(NORMAL_OFFSET) - NORMAL_OFFSET/2.f;
	ynorm += random(NORMAL_OFFSET) - NORMAL_OFFSET/2.f;
	znorm += random(NORMAL_OFFSET) - NORMAL_OFFSET/2.f;
	float m = sqrt(xnorm*xnorm + ynorm*ynorm +znorm*znorm);
	if (m!=0.f)
		m = 1.f/m;
	else
		m = 1.f;
	xnorm*=m;
	ynorm*=m;
	znorm*=m;
	glNormal3f(xnorm,ynorm,znorm);
	glVertex3f(x,y,z);
}

static void block_up(int x1, int y1, int z1, float uv, int tex_id)
{
	int x2, y2, z2;
	x2=x1+BLOCK_SIZE;
	y2=y1+BLOCK_SIZE;
	z2=z1;

	int xl,yl,zl;
	xl = x1>>5;
	yl = y1>>5;
	zl = z1>>5;

	float forward = block_at(state,xl,yl-1,zl)*0.5;
	float left = block_at(state,xl-1,yl,zl)*0.5;
	float right = block_at(state,xl+1,yl,zl)*0.5;
	float back = block_at(state,xl,yl+1,zl)*0.5;

	glBegin(GL_TRIANGLE_FAN);
		vertex(x1,y1,z2,0.f,0.f,1.f,uv*0.5,    uv*0.5,1.f,tex_id,1.f - (forward + left));
		vertex(x2,y1,z2,0.f,0.f,1.f,uv*0.5,    uv*0.5+0.5,1.f,tex_id,1.f - (forward + right));
		vertex(x2,y2,z2,0.f,0.f,1.f,uv*0.5+0.5,uv*0.5+0.5,1.f,tex_id,1.f - (back + right));
		vertex(x1,y2,z2,0.f,0.f,1.f,uv*0.5+0.5,uv*0.5,1.f,tex_id,1.f - (back + left));
	glEnd();
}
	

static void block_down(int x1, int y1, int z1, float uv, int tex_id)
{
	int x2, y2, z2;
	x2=x1+BLOCK_SIZE;
	y2=y1+BLOCK_SIZE;
	z2=z1;

	int xl,yl,zl;
	xl = x1>>5;
	yl = y1>>5;
	zl = z1>>5;

	float forward = block_at(state,xl,yl-1,zl-1)*0.5;
	float left = block_at(state,xl-1,yl,zl-1)*0.5;
	float right = block_at(state,xl+1,yl,zl-1)*0.5;
	float back = block_at(state,xl,yl+1,zl-1)*0.5;

	glBegin(GL_TRIANGLE_FAN);
		vertex(x2,y2,z2,0.f,0.f,-1.f,uv*0.5+0.5,uv*0.5    ,1.f,tex_id, 1.f - (back + right));
		vertex(x2,y1,z2,0.f,0.f,-1.f,uv*0.5,    uv*0.5    ,1.f,tex_id, 1.f - (forward + right));
		vertex(x1,y1,z2,0.f,0.f,-1.f,uv*0.5,    uv*0.5+0.5,1.f,tex_id, 1.f - (forward + left));
		vertex(x1,y2,z2,0.f,0.f,-1.f,uv*0.5+0.5,uv*0.5+0.5,1.f,tex_id, 1.f - (back + left));
	glEnd();
}

static void block_left(int x1, int y1, int z1, float uv, int tex_id)
{
	int y2, z2;
	//x2=x1+BLOCK_SIZE;
	y2=y1+BLOCK_SIZE;
	z2=z1-BLOCK_SIZE;

	int xl,yl,zl;
	xl = x1>>5;
	yl = y1>>5;
	zl = z1>>5;

	float forward = block_at(state,xl-1,yl-1,zl-1)*0.5;
	float up =    block_at(state,xl-1,yl,zl)*0.5;
	float down =   block_at(state,xl-1,yl,zl-2)*0.5;
	float back =    block_at(state,xl-1,yl+1,zl-1)*0.5;


	glBegin(GL_TRIANGLE_FAN);
		vertex(x1,y1,z2,-1.f,0.f,0.f,uv*0.5+0.5,uv*0.5+0.5,0.5f,tex_id, 1.f - down - forward);
		vertex(x1,y1,z1,-1.f,0.f,0.f,uv*0.5+0.5,uv*0.5    ,0.5f,tex_id, 1.f - up - forward);
		vertex(x1,y2,z1,-1.f,0.f,0.f,uv*0.5,	uv*0.5    ,0.5f,tex_id, 1.f - up - back);
		vertex(x1,y2,z2,-1.f,0.f,0.f,uv*0.5,	uv*0.5+0.5,0.5f,tex_id, 1.f - down - back);
	glEnd();
}

static void block_right(int x1, int y1, int z1, float uv, int tex_id)
{
	int x2, y2, z2;
	x2=x1+BLOCK_SIZE;
	y2=y1+BLOCK_SIZE;
	z2=z1-BLOCK_SIZE;

	int xl,yl,zl;
	xl = x1>>5;
	yl = y1>>5;
	zl = z1>>5;

	float forward = block_at(state,xl+1,yl-1,zl-1)*0.5;
	float up =    block_at(state,xl+1,yl,zl)*0.5;
	float down =   block_at(state,xl+1,yl,zl-2)*0.5;
	float back =    block_at(state,xl+1,yl+1,zl-1)*0.5;

	glBegin(GL_TRIANGLE_FAN);
		vertex(x2,y2,z2,1.f,0.f,0.f,uv*0.5+0.5,uv*0.5+0.5,0.5f,tex_id, 1.f - down - back);
		vertex(x2,y2,z1,1.f,0.f,0.f,uv*0.5+0.5,uv*0.5,0.5f,tex_id, 1.f - up - back);
		vertex(x2,y1,z1,1.f,0.f,0.f,uv*0.5,uv*0.5,0.5f,tex_id, 1.f - up - forward);
		vertex(x2,y1,z2,1.f,0.f,0.f,uv*0.5,uv*0.5+0.5,0.5f,tex_id, 1.f - down - forward);
	glEnd();
}

static void block_forward(int x1, int y1, int z1, int diag, int top, int bottom, int tex_id)
{
	int x2,z2;
	x2 = x1+BLOCK_SIZE;
	//y2 = y1+BLOCK_SIZE;
	z2 = z1-BLOCK_SIZE;

	float tlx,tly,trx,try,blx,bly,brx,bry;
	tlx = 0.5; tly = 0.5;	trx = 1.0; try = 0.5;
	blx = 0.5; bly = 1.0;	brx = 1.0; bry = 1.0;
	if (diag || (top && bottom))
	{
	    tlx = 0.0; tly = 0.0;	trx = 0.5; try = 0.0;
	    blx = 0.0; bly = 0.5;	brx = 0.5; bry = 0.5;
	}
	else
	{
	    if (top)
	    {
	        tlx = 0.5; tly = 0.0;	trx = 1.0; try = 0.0;
	        blx = 0.5; bly = 0.5;	brx = 1.0; bry = 0.5;
	    }
	    
	    else if (bottom)
	    {
	    	tlx = 0.0; tly = 0.5;	trx = 0.5; try = 0.5;
	       	blx = 0.0; bly = 1.0;	brx = 0.5; bry = 1.0;	
	    }	
	}

	int xl,yl,zl;
	xl = x1>>5;
	yl = y1>>5;
	zl = z1>>5;

	float left = block_at(state,xl-1  ,yl-1,  zl-1)*0.375;
	float up =      block_at(state,xl,yl-1, zl  )*0.375;
	float down =    block_at(state,xl,yl-1, zl-2)*0.375;
	float right =   block_at(state,xl+1,yl-1, zl-1)*0.375;
	
	glBegin(GL_TRIANGLE_FAN);
		vertex(x1,y1,z2,0.f,-1.f,0.f,blx,bly,0.f,tex_id, 0.75f - down - left);
	    vertex(x2,y1,z2,0.f,-1.f,0.f,brx,bry,0.f,tex_id, 0.75f - down - right);
	    vertex(x2,y1,z1,0.f,-1.f,0.f,trx,try,0.f,tex_id, 0.75f - up - right);
	    vertex(x1,y1,z1,0.f,-1.f,0.f,tlx,tly,0.f,tex_id, 0.75f - up - left);
	glEnd();
}

static void block_back(int x1, int y1, int z1, int diag, int top, int bottom, int tex_id)
{
	int x2,y2,z2;
	x2 = x1+BLOCK_SIZE;
	y2 = y1+BLOCK_SIZE;
	z2 = z1-BLOCK_SIZE;

	float tlx,tly,trx,try,blx,bly,brx,bry;
	tlx = 0.5; tly = 0.5;	trx = 1.0; try = 0.5;
	blx = 0.5; bly = 1.0;	brx = 1.0; bry = 1.0;
	if (diag || (top && bottom))
	{
	    tlx = 0.0; tly = 0.0;	trx = 0.5; try = 0.0;
	    blx = 0.0; bly = 0.5;	brx = 0.5; bry = 0.5;
	}
	else
	{
	    if (top)
	    {
	        tlx = 0.5; tly = 0.0;	trx = 1.0; try = 0.0;
	        blx = 0.5; bly = 0.5;	brx = 1.0; bry = 0.5;
	    }
	    
	    else if (bottom)
	    {
	    	tlx = 0.0; tly = 0.5;	trx = 0.5; try = 0.5;
	       	blx = 0.0; bly = 1.0;	brx = 0.5; bry = 1.0;	
	    }	
	}

	int xl,yl,zl;
	xl = x1>>5;
	yl = y1>>5;
	zl = z1>>5;

	float left = block_at(state,xl-1  ,yl+1,  zl-1)*0.375;
	float up =      block_at(state,xl,yl+1, zl  )*0.375;
	float down =    block_at(state,xl,yl+1, zl-2)*0.375;
	float right =   block_at(state,xl+1,yl+1, zl-1)*0.375;

	glBegin(GL_TRIANGLE_FAN);
		vertex(x2,y2,z1,0.f,1.f,0.f,trx,try,0.f,tex_id, 0.75f - up - right);
	    vertex(x2,y2,z2,0.f,1.f,0.f,brx,bry,0.f,tex_id, 0.75f - down - right);
	    vertex(x1,y2,z2,0.f,1.f,0.f,blx,bly,0.f,tex_id, 0.75f - down - left);
	    vertex(x1,y2,z1,0.f,1.f,0.f,tlx,tly,0.f,tex_id, 0.75f - up - left);
	glEnd();
}

int block_get_lit(game_state *state,int x, int y, int z)
{
	while (x < LEVEL_SIZE && z < LEVEL_SIZE)
	{
		x++;
		z++;
		if (block_at(state,x,y,z) )
			return 0;
	}
	return 1;
}

GLuint level_model_build_part(game_state *state,int start)
{
	GLuint list = glGenLists(1);
	glNewList(list,GL_COMPILE);
	int x,y,z,xb,yb,zb,lit,tex_id;
	// save rng state
	unsigned int seed = SEED;
	for(int i = start; i<start+CHUNK_SIZE && i<state->block_count;i++)
	{
		if (!(state->block_list[i]>>31 & 1))
		{
			seed_rng(state->block_list[i]);
			tex_id = irandom(6);
			x = point_getx(state->block_list[i]);
			y = point_gety(state->block_list[i]);
			z = point_getz(state->block_list[i]);

			xb = x << 5;
			yb = y << 5;
			zb = z << 5;

			lit = block_get_lit(state,x,y,z);

			// up
			if (!block_at(state,x,y,z+1))
				block_up(xb,yb,zb+BLOCK_SIZE,(float)(block_get_lit(state,x,y,z+1) && lit),tex_id);
			// left 
			if (!block_at(state,x-1,y,z))
				block_left(xb,yb,zb+BLOCK_SIZE,0.f,tex_id);
			// right
			if (!block_at(state,x+1,y,z))
				block_right(xb,yb,zb+BLOCK_SIZE,(float)(block_get_lit(state,x+1,y,z) && lit),tex_id);
			//forward
			if (!block_at(state,x,y-1,z))
				block_forward(xb,yb,zb+BLOCK_SIZE,!block_get_lit(state,x,y-1,z),!block_get_lit(state,x,y-1,z+1) || block_at(state,x,y-1,z+1),!block_get_lit(state,x,y-1,z-1),tex_id);
			//back
			if (!block_at(state,x,y+1,z))
				block_back(xb,yb,zb+BLOCK_SIZE,!block_get_lit(state,x,y+1,z),!block_get_lit(state,x,y+1,z+1) || block_at(state,x,y+1,z+1),!block_get_lit(state,x,y+1,z-1),tex_id);
			// down
			if (!block_at(state,x,y,z-1))
				block_down(xb,yb,zb,0.f,tex_id);
		}
	}
	// restore rng state
	seed_rng(seed);
	glEndList();
	return list;
}

void level_model_build(game_state *state)
{
	for(int i = 0; i<MAX_BLOCKS/CHUNK_SIZE; i++)
	{
		state->level_model[i] = level_model_build_part(state,i*CHUNK_SIZE);
	}
}

void level_model_destroy(game_state *state)
{
	for(int i = 0; i<MAX_BLOCKS/CHUNK_SIZE; i++)
	{
		model_destroy(state->level_model[i]);
	}
}

void level_model_draw(game_state *state)
{
	/************************************************************************************/
	/*
	v3 p = state->position[state->player];
	int playerBlock = point_create(
		((int)p.x)>>5 &1023,
		((int)p.y)>>5 &1023,
		((((int)p.z)>>5)-1)&1023
	);
	model_draw(state->level_model[getClosestBlockIndex(state,playerBlock)/CHUNK_SIZE]);
	*/
	/***************************************************************************************/
	for(int i = 0; i<MAX_BLOCKS/CHUNK_SIZE; i++)
	{
		model_draw(state->level_model[i]);
	}
}

void draw_closest_block_pos(game_state *state)
{
	v3 p = state->position[state->player];
	int playerBlock = point_create(
		((int)p.x)>>5 &1023,
		((int)p.y)>>5 &1023,
		((((int)p.z)>>5)-1)&1023
	);

	int block = getClosestBlock(state,playerBlock);
	float x = (float) point_getx(block)*32.f;
	float y = (float) point_gety(block)*32.f;
	float z = (float) point_getz(block)*32.f;
	drawSphere(x,y,z,64,8);
}

void gvertex(float x, float y, float z, float uvx, float uvy, int tex_id)
{
	float idx = (float)(tex_id & 0x1f);
	float idy = (float)((tex_id >> 5 ) & 0x1f);
	glTexCoord2f((uvx+idx)/32.f,(uvy+idy)/32.f);
	glVertex3f(x,y,z);
}

GLuint grass_model_build_part(game_state *state,int start)
{
	GLuint list = glGenLists(1);
	glNewList(list,GL_COMPILE);
	unsigned int seed = SEED;
	int x,y,z,xb,yb,zb,range,tex_id;
	
	for(int i = start; i<start+CHUNK_SIZE;i++)
	{
		if (!(state->block_list[i]>>31 & 1))
		{
			x = point_getx(state->block_list[i]);
			y = point_gety(state->block_list[i]);
			z = point_getz(state->block_list[i]);
			
			xb = x << 5;
			yb = y << 5;
			zb = z << 5;
			seed_rng(point_create(x,y,z));
			range = 16.f+random(16.f);
			tex_id = irandom(8);
			
			if (irandom(10)>5 && !block_at(state,x,y,z+1))
			{
				int count = 8;
				glBegin(GL_QUADS);
				for(int w = 0; w<count; w++)
				{
					int gx, gy, gz,gx2,gy2,gz2;
					float gsize = 16.f;
					float dir = random(360.f);
					gx = xb + random(32);
					gy = yb + random(32);
					gz = zb + 32.f;
					gx2 = gx + random(range) - range/2.f;
					gy2 = gy + random(range) - range/2.f;
					gz2 = gz + 32.f;

					float xsize = lengthdir_x(gsize,dir);
					float ysize = lengthdir_y(gsize,dir);
					
					gvertex(gx-xsize,gy-ysize,gz,0.f,1.f,tex_id);
					gvertex(gx2-xsize,gy2-ysize,gz2,0.f,0.f,tex_id);
					gvertex(gx2+xsize,gy2+ysize,gz2,1.f,0.f,tex_id);
					gvertex(gx+xsize,gy+ysize,gz,1.f,1.f,tex_id);
				}
				glEnd();
			}
		}
	}
	
	seed_rng(seed);
	glEndList();
	return list;
}

void grass_model_build(game_state *state)
{
	for(int i = 0; i<MAX_BLOCKS/CHUNK_SIZE; i++)
	{
		state->grass_model[i] = grass_model_build_part(state,i*CHUNK_SIZE);
	}
}

void grass_model_destroy(game_state *state)
{
	for(int i = 0; i<MAX_BLOCKS/CHUNK_SIZE; i++)
	{
		model_destroy(state->grass_model[i]);
	}
}

void grass_model_draw(game_state *state)
{
	for(int i = 0; i<MAX_BLOCKS/CHUNK_SIZE; i++)
	{
		model_draw(state->grass_model[i]);
	}
}

GLuint dust_model_build(game_state *state)
{
	GLuint list = glGenLists(1);
	int x,y,z,xb,yb,zb;
	glNewList(list,GL_COMPILE);
	glBegin(GL_POINTS);
	for(int i = 0; i<state->block_count;i++)
	{
		x = point_getx(state->block_list[i]);
		y = point_gety(state->block_list[i]);
		z = point_getz(state->block_list[i]);

		xb = x << 5;
		yb = y << 5;
		zb = z << 5;
		if (irandom(2)==0)
			nvertex(xb+irandom(1024)-512,yb+irandom(1024)-512,zb+irandom(1024)-512,-0.70710678118,0,0.70710678118);
	}
	glEnd();
	glEndList();
	return list;
}

void model_draw(GLuint model)
{
	glCallList(model);
}

void model_destroy(GLuint model)
{
	glDeleteLists(model,1);
}

GLuint texture_load(const char *file, int width, int height)
{
	int n = 4;
	int w = width;
	int h = height;
	unsigned char *data = stbi_load(file,&w,&h,&n,0);
	GLuint tid;
	glGenTextures(1,&tid);
	glBindTexture(GL_TEXTURE_2D,tid);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D,0);
    free(data);
    return tid;
}

GLuint texture_load_mipmapped(const char *file, int width, int height)
{
	int n = 4;
	int w = width;
	int h = height;
	unsigned char *data = stbi_load(file,&w,&h,&n,0);
	GLuint tid;
	glGenTextures(1,&tid);
	glBindTexture(GL_TEXTURE_2D,tid);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
    glBindTexture(GL_TEXTURE_2D,0);
    free(data);
    return tid;
}

void texture_destroy(GLuint tex)
{
	glDeleteTextures(1,&tex);
}

void sprite_add(game_state *state, int entity, float sprite_index, float image_count, float width, float height)
{
	entity_component_add(state,entity,c_sprite);
	spr *sprite = &(state->sprite[entity]);

	sprite->sprite_index = sprite_index;
	sprite->image_count = image_count;
	sprite->width = width;
	sprite->height = height;
	sprite->qwidth = width;
	sprite->qheight = height;
	sprite->image_index = 0;
	sprite->image_speed = 0.f;
	sprite->play_once = 0.f;
}

void sprite_add_size(game_state *state, int entity, float sprite_index,float image_count, float width, float height, float qwidth, float qheight)
{
	sprite_add(state,entity,sprite_index,image_count,width,height);
	spr *sprite = &(state->sprite[entity]);
	sprite->qwidth = qwidth;
	sprite->qheight = qheight;
}

static float mdist(game_state *state, float x, float y, float z)
{
	float diffx = abs(state->camx - x);
	float diffy = abs(state->camy - y);
	float diffz = abs(state->camz - z);

	return diffx+diffy+diffz;
}

static int str_size(const char *str)
{
	int count = 0;
	char *c = (char *)str;
	while(*c)
	{	
		c++;
		count++;
	}
	return count;
}

void draw_text_br(float xstart, float ystart, float size, const char *text)
{
	float xoffset = str_size(text)*size;
	float yoffset = size;

	glPushMatrix();
	glTranslatef(-xoffset,-yoffset,0);
	draw_text(xstart,ystart,size,text);
	glPopMatrix();
}

void draw_text(float xstart, float ystart, float size, const char *text)
{
	char *c = (char *)text;
	while(*c)
	{
		float x = (float)(*c - 32)*8.f;
		float y = 1016.f;
		float xto = x+8.f;
		float yto = y+8.f;
		x/=1024.f;
		y/=1024.f;
		xto/=1024.f;
		yto/=1024.f;
		

		if (*c>31 && *c<123)
		{
			glBegin(GL_TRIANGLE_FAN);
			glTexCoord2f(xto,y);
			glVertex3f(xstart+size,ystart,0);
			glTexCoord2f(x,y);
			glVertex3f(xstart,ystart,0);
			glTexCoord2f(x,yto);
			glVertex3f(xstart,ystart+size,0);
			glTexCoord2f(xto,yto);
			glVertex3f(xstart+size,ystart+size,0);
			glEnd();
		}
		xstart+=size;
		c++;
	}
}

void draw_player_gun(game_state *state)
{
	if (state->player == -1)
		return;

	v3 *p = &(state->position[state->player]);
	gun *g = &(state->pstate.weapons[state->pstate.weapon]);
	// gun size is 32 on sprite sheet
	float x = g->sprite * 32.f;
	float y = 928.f;
	float xto = x+32.f;
	float yto = y+32.f;
	float width = 4.f;
	float height = 12.f;


	/*
	float xpos = 8.5 - g->recoil*0.3333f;
	float ypos = -7;
	float zpos = -2;*/

	float xpos = 7.5 - g->recoil*(0.3333f) - sin(g->reload)*5.f;
	float ypos = -8.5;
	float zpos = -3;

	x/=1024.f;
	y/=1024.f;
	xto/=1024.f;
	yto/=1024.f;
	glDepthFunc(GL_ALWAYS);
	glPushMatrix();
		//glRotatef(state->camzdir,0.f,1.f,0.f);
		//
		glTranslatef(p->x,p->y,p->z+state->vheight+sin(state->view_bob)*2.5f);
		glPushMatrix();
			glRotatef(state->gundir,0.f,0.f,-1.f);
			glRotatef(state->gunzdir + g->recoil/1.5f,0.f,-1.f,0.f);
			glScalef(state->gun_change,1,1);
			glColor3f(0.f,0.f,0.f);
			glPushMatrix();
				glTranslatef(xpos,ypos-0.5,zpos);
				glBegin(GL_TRIANGLE_FAN);
				glTexCoord2f(xto,y);
			   	glVertex3f(-width,0,height);
				glTexCoord2f(x,y);
				glVertex3f(width,0,height);
				glTexCoord2f(x,yto);
				glVertex3f(width,0,-height);
				glTexCoord2f(xto,yto);
				glVertex3f(-width,0,-height);
				glEnd();
			glPopMatrix();
			glColor3f(state->in_shadow,state->in_shadow,state->in_shadow);
			glPushMatrix();
				glTranslatef(xpos,ypos,zpos);
				glBegin(GL_TRIANGLE_FAN);
				glTexCoord2f(xto,y);
			   	glVertex3f(-width,0,height);
				glTexCoord2f(x,y);
				glVertex3f(width,0,height);
				glTexCoord2f(x,yto);
				glVertex3f(width,0,-height);
				glTexCoord2f(xto,yto);
				glVertex3f(-width,0,-height);
				glEnd();
			glPopMatrix();
			glColor3f(1.f,1.f,1.f);
		glPopMatrix();
	glPopMatrix();
	glDepthFunc(GL_LESS);
	//glEnable(GL_DEPTH_TEST);
}

void draw_sprite(game_state *state, int entity)
{
	spr *sprite = &(state->sprite[entity]);
	v3 *pos = &(state->position[entity]);

	float x = floor(sprite->image_index)*sprite->width;
	float y = sprite->sprite_index*sprite->height;
	float xto = x+sprite->width;
	float yto = y+sprite->height;
	float width = sprite->qwidth;
	float height = sprite->qheight;
	x/=1024.f;
	y/=1024.f;
	xto/=1024.f;
	yto/=1024.f;
	const float maxdist = 1024.f;
	float dist = 1.f;
	float red = 1.f;
	float green = 1.f;
	float blue = 1.f;
	if (!entity_has_component(state,entity,c_sprite_fullbright))
	{
		dist = 1.f - clamp(mdist(state,pos->x,pos->y,pos->z),0.f,maxdist)/maxdist;
		red = lerp(state->levelFogColor[0],1.f,dist);
		green = lerp(state->levelFogColor[1],1.f,dist);
		blue = lerp(state->levelFogColor[2],1.f,dist);
	}

	glPushMatrix();
		glTranslatef(pos->x,pos->y,pos->z);
		glPushMatrix();
			GLfloat m[4][4];
			glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)m);
			m[0][0] = 1.f;
			m[0][1] = 0.f;
			m[0][2] = 0.f;

			m[1][0] = 0.f;
			m[1][1] = 1.f;
			m[1][2] = 0.f;

			m[2][0] = 0.f;
			m[2][1] = 0.f;
			m[2][2] = 1.f;
			glLoadMatrixf(&m[0][0]);
			glColor3f(red,green,blue);
			glBegin(GL_TRIANGLE_FAN);
			glTexCoord2f(xto,y);
		   	glVertex3f(width,height,0);
		   	glTexCoord2f(x,y);
		   	glVertex3f(-width,height,0);
		   	glTexCoord2f(x,yto);
		    glVertex3f(-width,-height,0);
		    glTexCoord2f(xto,yto);
		    glVertex3f(width,-height,0);
		    glEnd();
		    if (entity_has_component(state,entity,c_sprite_background))
		    {
		    	glColor3f(0.f,0.f,0.f);
		    	glBegin(GL_TRIANGLE_FAN);
		    	glTexCoord2f(xto,y);
		   		glVertex3f(width,height,-1.f);
		   		glTexCoord2f(x,y);
		   		glVertex3f(-width,height,-1.f);
		   		glTexCoord2f(x,yto);
		    	glVertex3f(-width,-height,-1.f);
		    	glTexCoord2f(xto,yto);
		    	glVertex3f(width,-height,-1.f);
		    	glEnd();
		    }
		    glColor3f(1.f,1.f,1.f);
			
		glPopMatrix();
	glPopMatrix();
}

void drawSphere(float x, float y, float z, float r, int sides) 
{
	int i, j;
	for(i = 0; i <= sides; i++) 
	{
		float lat0 = PI * (-0.5 + (float) (i - 1) / sides);
		float z0  = sin(lat0)*r;
		float zr0 =  cos(lat0)*r;

		float lat1 = PI * (-0.5 + (float) i / sides);
		float z1 = sin(lat1)*r;
		float zr1 = cos(lat1)*r;
		glPushMatrix();
			glTranslatef(x,y,z);
				glBegin(GL_QUAD_STRIP);
				for(j = 0; j <= sides; j++) 
				{
					float lng = 2 * PI * (float) (j - 1) / sides;
					float x = cos(lng);
					float y = sin(lng);

					glVertex3f(x * zr0, y * zr0, z0);
					glVertex3f(x * zr1, y * zr1, z1);
				}
				glEnd();
		glPopMatrix();
	}
}

static void drawLightBall(game_state *state, int ent)
{
	v3 *p = &(state->position[ent]);
	pLight *l = &(state->lights[ent]);
	drawSphere(p->x,p->y,p->z,l->radius,8);
}

static void drawLightBalls(game_state *state)
{
	int *ents=get_ec_set(state,c_light);
	for(int i=0; iterate_ec_set(ents,i); i++)
		drawLightBall(state,ents[i]);
}

static void ProjectLights(game_state *state)
{
	int i;
	v3 *pos = &(state->position[state->player]);
	int *ents=get_ec_set(state,c_light);
	for(i=0; iterate_ec_set(ents,i); i++)
		state->lights[ents[i]].distSquared = distanceSquared(pos,&(state->position[ents[i]]));
	for(i=0; i<state->comp_count[c_light]-1; i++)
	{
		if (state->lights[ents[i]].distSquared > state->lights[ents[i+1]].distSquared)
			ec_set_swap(state,c_light,i,i+1);
	}

	int c = 0;
	for(i=0; iterate_ec_set(ents,i); i++)
	{
		v3 *pos = &(state->position[ents[i]]);
		pLight *light = &(state->lights[ents[i]]);
		GLuint lid = LightID[c];

		glLightfv(lid, GL_AMBIENT, (float [4]){0.f,0.f,0.f,1.f});
		glLightfv(lid, GL_DIFFUSE, state->levelColor);
		glLightf(lid, GL_CONSTANT_ATTENUATION, 1.f);
		glLightf(lid, GL_LINEAR_ATTENUATION, 0.0f);
		//float polyLight = 256.f;
		glLightf(lid, GL_QUADRATIC_ATTENUATION,1.f/(light->attenuation*light->attenuation));

		GLfloat light_pos[4];
		light_pos[0] = pos->x;
		light_pos[1] = pos->y;
		light_pos[2] = pos->z;
		light_pos[3] = 1.f;
		glLightfv(lid,GL_POSITION,light_pos);
		glLightfv(lid,GL_SPECULAR,(float [4]){0.f,0.f,0.f,0.f});
		glEnable(lid);
		c++;
		if (c>=7)
			break;
	}	
}

void load_screen_draw(int time)
{
	extern load_state LOAD_STATE;
	extern SDL_Window *window;
	while (time > 0)
	{
		time--;
		LOAD_STATE.progress++;
		//unsigned long int ltime = SDL_GetPerformanceCounter()*1000/SDL_GetPerformanceFrequency();
		//float c = random(0.2f);
		//glClearColor(c,c,c,1.f);
		glClear(GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0,105,105,0,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glColor3f(1.f,1.f,1.f);
		glBegin(GL_LINES);
		glVertex2i(0,0);
		glVertex2i(LOAD_STATE.progress/2,LOAD_STATE.progress/2);

		glVertex2i(0,105);
		glVertex2i(LOAD_STATE.progress/2,105-LOAD_STATE.progress/2);

		glVertex2i(105,0);
		glVertex2i(105-LOAD_STATE.progress/2,LOAD_STATE.progress/2);

		glVertex2i(105,105);
		glVertex2i(105-LOAD_STATE.progress/2,105-LOAD_STATE.progress/2);
		glEnd();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glColor4f(0.f,0.f,0.f,0.2f);
		glBegin(GL_TRIANGLE_FAN);
		glVertex2i(0,105);
		glVertex2i(105,105);
		glVertex2i(105,0);
		glVertex2i(0,0);
		glEnd();
		glDisable(GL_BLEND);

		SDL_GL_SwapWindow(window);
		SDL_Delay(1);

	}
}

float tex_id_x(int id)
{
	return (float)(id & 0xf);
}

float tex_id_y(int id)
{
	return (float)((id>>4) & 0xf);
}