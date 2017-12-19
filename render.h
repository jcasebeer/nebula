#ifndef RENDER_H
#define RENDER_H
#include <SDL2/SDL_opengl.h>
#include "state.h"

//turn off shaders w/ this
//#define NO_SHADER

typedef struct texture_data
{
	GLuint sprites;
	GLuint shadow_1024;
	GLuint grass;
}texture_data;

typedef struct surface_data
{
	int width;
	int height;
	float gamma;
	int lines;
	GLuint fbo;
	GLuint fbo_texture;
	GLuint rbo_depth;
	GLuint vbo_fbo_verts;
	GLuint post_shader;
	GLuint u_fbo_texture;
	GLuint a_vcoord;
	GLuint u_gamma;
	GLuint u_lines;
	GLuint u_resx;
	GLuint u_resy;
}surface_data;


typedef struct
{
	int progress;
}load_state;

texture_data *texture_data_create();
void draw_position_camera(float x, float y, float z, float xto, float yto, float zto);
void draw_set_frustum(float fov, float ar,float znear, float zfar);
void level_model_build(game_state *state);
void model_draw(GLuint model);
void model_destroy(GLuint model);
GLuint texture_load(const char *file, int width, int height);
GLuint texture_load_mipmapped(const char *file, int width, int height);
void texture_destroy(GLuint texture);
surface_data *surface_data_create(int width, int height, float gamma);
void surface_data_destroy(surface_data *surf);
void game_render(game_state *state, SDL_Window *window, texture_data *textures);
void game_render_pp(game_state *state, SDL_Window *window, texture_data *textures, surface_data *surf);
void draw_sprite(game_state *state, int entity);
GLuint grass_model_build_part(game_state *state, int start);
void grass_model_destroy(game_state *state);
void grass_model_draw(game_state *state);
void grass_model_build(game_state *state);
GLuint dust_model_build(game_state *state);
GLuint level_model_build_part(game_state *state,int start);
void level_model_destroy(game_state *state);
void level_model_draw(game_state *state);
void load_screen_draw(int time);
void draw_hud(game_state *state, SDL_Window *window, texture_data *textures);
void drawSphere(float x, float y, float z, float r, int sides);
#endif