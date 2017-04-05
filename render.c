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

surface_data *surface_data_create(int width, int height)
{
	surface_data *surf = malloc(sizeof(surface_data));

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

	glUseProgram(surf->post_shader);

	//glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,surf->fbo_texture);
	glUniform1i(surf->u_fbo_texture,0);
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
	//glDisable(GL_TEXTURE_2D);
}

void game_render(game_state *state, SDL_Window *window, texture_data *textures)
{
	// clear background
	//glShadeModel(GL_FLAT);
	glClearColor(0.1f,0.1f,0.1f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set up projection matrix and window size
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	int width, height;
	SDL_GetWindowSize(window, &width, &height);
	glViewport(0,0,width,height);
	draw_set_frustum(90.f,(float)width/height,1.,32000.);

	// point camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
		
	draw_position_camera(
		state->camx,
        state->camy,
        state->camz+state->vheight,
        state->camx+lengthdir_x(lengthdir_x(1,-state->camzdir),state->camdir),
        state->camy+lengthdir_y(lengthdir_x(1,-state->camzdir),state->camdir),
        state->camz+state->vheight+lengthdir_y(1,-state->camzdir)
	);

	// draw level model
	//glPointSize(2.0);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glPushMatrix();
	
	GLfloat AmbientGlobal[4] = {0.0f,0.0f,0.0f,1.f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,AmbientGlobal);

	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.f);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0f);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION,1./(384.0*384.0));
	//glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION,0.f);

	GLfloat spec[4] = {0.f,0.f,0.f,1.f};

	GLfloat light_pos[4];
	light_pos[0] = state->camx;
	light_pos[1] = state->camy;
	light_pos[2] = state->camz;
	light_pos[3] = 1.f;
	glLightfv(GL_LIGHT0,GL_POSITION,light_pos);
	glLightfv(GL_LIGHT0,GL_SPECULAR,spec);
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,textures->shadow);
	model_draw(state->level_model);
	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_TEXTURE_2D);

	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glPopMatrix();
	

	glAlphaFunc(GL_GREATER,0.f);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_TEXTURE_2D);

	// test drawing textures
	//glDisable(GL_CULL_FACE);
	glBindTexture(GL_TEXTURE_2D,textures->sprites);
	int *sprites = get_ec_set(state,c_sprite);
	for(int i = 0; iterate_ec_set(sprites,i); i++)
	{
		draw_sprite(state,sprites[i]);
	}

	glBindTexture(GL_TEXTURE_2D,0);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_TEXTURE_2D);
}

texture_data *texture_data_create()
{
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
	return irandom(amount) - (amount >> 1);
}

static void vertex(int x, int y, int z, float xnorm, float ynorm, float znorm, float uv_x, float uv_y)
{
	seed_rng(x*y-z);
	glTexCoord2f(uv_x,uv_y);
	glNormal3f(xnorm,ynorm,znorm);
	glVertex3i(x+offset(6),y+offset(6),z+offset(6));
}

static void block_up(int x1, int y1, int z1, float uv)
{
	int x2, y2, z2;
	x2=x1+BLOCK_SIZE;
	y2=y1+BLOCK_SIZE;
	z2=z1;

	glBegin(GL_TRIANGLE_FAN);
		vertex(x1,y2,z2,0.f,0.f,1.f,uv,uv);
		vertex(x1,y1,z2,0.f,0.f,1.f,uv,uv);
		vertex(x2,y1,z2,0.f,0.f,1.f,uv,uv);
		vertex(x2,y2,z2,0.f,0.f,1.f,uv,uv);
	glEnd();
}

static void block_down(int x1, int y1, int z1, float uv)
{
	int x2, y2, z2;
	x2=x1+BLOCK_SIZE;
	y2=y1+BLOCK_SIZE;
	z2=z1;

	glBegin(GL_TRIANGLE_FAN);
		vertex(x2,y2,z2,0.f,0.f,-1.f,uv,uv);
		vertex(x2,y1,z2,0.f,0.f,-1.f,uv,uv);
		vertex(x1,y1,z2,0.f,0.f,-1.f,uv,uv);
		vertex(x1,y2,z2,0.f,0.f,-1.f,uv,uv);
	glEnd();
}

static void block_left(int x1, int y1, int z1, float uv)
{
	int y2, z2;
	//x2=x1+BLOCK_SIZE;
	y2=y1+BLOCK_SIZE;
	z2=z1-BLOCK_SIZE;

	glBegin(GL_TRIANGLE_FAN);
		vertex(x1,y1,z2,-1.f,0.f,0.f,uv,uv);
		vertex(x1,y1,z1,-1.f,0.f,0.f,uv,uv);
		vertex(x1,y2,z1,-1.f,0.f,0.f,uv,uv);
		vertex(x1,y2,z2,-1.f,0.f,0.f,uv,uv);
	glEnd();
}

static void block_right(int x1, int y1, int z1, float uv)
{
	int x2, y2, z2;
	x2=x1+BLOCK_SIZE;
	y2=y1+BLOCK_SIZE;
	z2=z1-BLOCK_SIZE;

	glBegin(GL_TRIANGLE_FAN);
		vertex(x2,y2,z2,1.f,0.f,0.f,uv,uv);
		vertex(x2,y2,z1,1.f,0.f,0.f,uv,uv);
		vertex(x2,y1,z1,1.f,0.f,0.f,uv,uv);
		vertex(x2,y1,z2,1.f,0.f,0.f,uv,uv);
	glEnd();
}

static void block_forward(int x1, int y1, int z1, int diag, int top, int bottom)
{
	int x2,z2;
	x2 = x1+BLOCK_SIZE;
	//y2 = y1+BLOCK_SIZE;
	z2 = z1-BLOCK_SIZE;

	int tlx,tly,trx,try,blx,bly,brx,bry;
	tlx = 1; tly = 1;
	trx = 1; try = 1;
	blx = 1; bly = 1;
	brx = 1; bry = 1;
	if (diag || (top && bottom))
	{
	    tlx = 0; tly = 0;
	    trx = 0; try = 0;
	    blx = 0; bly = 0;
	    brx = 0; bry = 0;
	}
	else
	{
	    if (top)
	    {
	        trx = 0; try = 1;
	        tlx = 0; tly = 0;
	        brx = 1; bry = 1;
	        blx = 1; bly = 0;
	    }
	    else if (bottom)
	    {
	        trx = 1; try = 0;
	        tlx = 1; tly = 1;
	        brx = 0; bry = 0;
	        blx = 0; bly = 1;
	    }
	}

	glBegin(GL_TRIANGLE_FAN);
		vertex(x1,y1,z2,0.f,-1.f,0.f,blx,bly);
	    vertex(x2,y1,z2,0.f,-1.f,0.f,brx,bry);
	    vertex(x2,y1,z1,0.f,-1.f,0.f,trx,try);
	    vertex(x1,y1,z1,0.f,-1.f,0.f,tlx,tly);
	glEnd();
}

static void block_back(int x1, int y1, int z1, int diag, int top, int bottom)
{
	int x2,y2,z2;
	x2 = x1+BLOCK_SIZE;
	y2 = y1+BLOCK_SIZE;
	z2 = z1-BLOCK_SIZE;

	int tlx,tly,trx,try,blx,bly,brx,bry;
	tlx = 1; tly = 1;
	trx = 1; try = 1;
	blx = 1; bly = 1;
	brx = 1; bry = 1;
	if (diag || (top && bottom))
	{
	    tlx = 0; tly = 0;
	    trx = 0; try = 0;
	    blx = 0; bly = 0;
	    brx = 0; bry = 0;
	}
	else
	{
	    if (top)
	    {
	        trx = 0; try = 1;
	        tlx = 0; tly = 0;
	        brx = 1; bry = 1;
	        blx = 1; bly = 0;
	    }
	    else if (bottom)
	    {
	        trx = 1; try = 0;
	        tlx = 1; tly = 1;
	        brx = 0; bry = 0;
	        blx = 0; bly = 1;
	    }
	}

	glBegin(GL_TRIANGLE_FAN);
		vertex(x2,y2,z1,0.f,1.f,0.f,blx,bly);
	    vertex(x2,y2,z2,0.f,1.f,0.f,brx,bry);
	    vertex(x1,y2,z2,0.f,1.f,0.f,trx,try);
	    vertex(x1,y2,z1,0.f,1.f,0.f,tlx,tly);
	glEnd();
}

static int block_get_lit(game_state *state,int x, int y, int z)
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


GLuint level_model_build(game_state *state)
{
	GLuint list = glGenLists(1);
	glNewList(list,GL_COMPILE);
	int x,y,z,xb,yb,zb,lit;
	// save rng state
	unsigned int seed = SEED;
	for(int i = 0; i<state->block_count;i++)
	{
		x = point_getx(state->block_list[i]);
		y = point_gety(state->block_list[i]);
		z = point_getz(state->block_list[i]);

		xb = x << 5;
		yb = y << 5;
		zb = z << 5;

		lit = block_get_lit(state,x,y,z);

		// up
		if (!block_at(state,x,y,z+1))
			block_up(xb,yb,zb+BLOCK_SIZE,(float)(block_get_lit(state,x,y,z+1) && lit));
		// left 
		if (!block_at(state,x-1,y,z))
			block_left(xb,yb,zb+BLOCK_SIZE,0.f);
		// right
		if (!block_at(state,x+1,y,z))
			block_right(xb,yb,zb+BLOCK_SIZE,(float)(block_get_lit(state,x+1,y,z) && lit));
		//forward
		if (!block_at(state,x,y-1,z))
			block_forward(xb,yb,zb+BLOCK_SIZE,!block_get_lit(state,x,y-1,z),!block_get_lit(state,x,y-1,z+1) || block_at(state,x,y-1,z+1),!block_get_lit(state,x,y-1,z-1));
		//back
		if (!block_at(state,x,y+1,z))
			block_back(xb,yb,zb+BLOCK_SIZE,!block_get_lit(state,x,y+1,z),!block_get_lit(state,x,y+1,z+1) || block_at(state,x,y+1,z+1),!block_get_lit(state,x,y+1,z-1));
		// down
		if (!block_at(state,x,y,z-1))
			block_down(xb,yb,zb,0.f);
	}
	// restore rng state
	seed_rng(seed);
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
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

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
	sprite->image_index = 0;
	sprite->image_speed = 0.f;
}

static float mdist(game_state *state, float x, float y, float z)
{
	float diffx = abs(state->camx - x);
	float diffy = abs(state->camy - y);
	float diffz = abs(state->camz - z);

	return diffx+diffy+diffz;
}

void draw_sprite(game_state *state, int entity)
{
	spr *sprite = &(state->sprite[entity]);
	v3 *pos = &(state->position[entity]);

	float x = floor(sprite->image_index)*sprite->width;
	float y = sprite->sprite_index*sprite->height;
	float xto = x+sprite->width;
	float yto = y+sprite->height;
	float width = sprite->width;
	float height = sprite->height;
	x/=1024.f;
	y/=1024.f;
	xto/=1024.f;
	yto/=1024.f;
	const float maxdist = 2048.f;
	float dist = 1.f - clamp(mdist(state,pos->x,pos->y,pos->z),0.f,maxdist)/maxdist;

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
			glBegin(GL_TRIANGLE_FAN);
			glColor3f(dist,dist,dist);
			glTexCoord2f(xto,y);
		   	glVertex3f(width,height,0);
		   	glTexCoord2f(x,y);
		   	glVertex3f(-width,height,0);
		   	glTexCoord2f(x,yto);
		    glVertex3f(-width,-height,0);
		    glTexCoord2f(xto,yto);
		    glVertex3f(width,-height,0);
		    glColor3f(1.f,1.f,1.f);
			glEnd();
		glPopMatrix();
	glPopMatrix();


	if (ceil(sprite->image_index) < sprite->image_count)
		sprite->image_index += sprite->image_speed;
	else
		sprite->image_index = 0;
}