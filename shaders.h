#ifndef SHADER_H
#define SHADER_H

const char *post_shader_v = ""
	"attribute vec2 v_coord;\n"
	"uniform sampler2D fbo_texture;\n"
	"varying vec2 f_texcoord;\n"

	"void main(void) {\n"
  		"gl_Position = vec4(v_coord, 0.0, 1.0);\n"
  		"f_texcoord = (v_coord + 1.0) / 2.0;\n"
	"}\n";

const char *post_shader_f = ""
	"uniform sampler2D fbo_texture;\n"
	"varying vec2 f_texcoord;\n"

	"void main(void) {\n"
  		"gl_FragColor = texture2D(fbo_texture, f_texcoord);\n"
	"}\n";

#endif