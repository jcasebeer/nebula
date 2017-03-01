#ifndef RENDER_H
#define RENDER_H
#include <SDL2/SDL_opengl.h>
#include "state.h"

void draw_position_camera(float x, float y, float z, float xto, float yto, float zto);
void draw_set_frustum(float fov, float ar,float znear, float zfar);
GLuint level_model_build(game_state *state);
void model_draw(GLuint model);
void model_destroy(GLuint model);
GLuint texture_load(const char *file, int width, int height);

#endif