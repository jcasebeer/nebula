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
	"uniform float gamma;"
	"uniform float resx;"
	"uniform float resy;"
	"varying vec2 f_texcoord;\n"


	"void main(void) {\n"
	"vec3 c = texture2D(fbo_texture, f_texcoord).rgb;\n"
	"vec2 uv = gl_FragCoord.xy / vec2(resx,resy);\n"
	"float xdist = 0.5 - uv.x;\n"
	"float ydist = 0.5 - uv.y;\n"
	"float m = 1.0 - sqrt(xdist*xdist+ydist*ydist);\n"
	"c.rgb = c.rgb*vec3(m,m,m);\n"
	"float r = c.r*256.0;\n"
	"float g = c.g*256.0;\n"
	"float b = c.b*256.0;\n"
	"gl_FragColor = vec4(floor(r/32.0)/8.0,floor(g/32.0)/8.0,floor(b/16.0)/16.0,1.0);\n"
//	"else\n"
	//"	gl_FragColor = vec4(c,1.0);\n"
//	"gl_FragColor.rgb = (gl_FragColor.rgb,vec3(gamma));\n"
	"}\n";

/*const char *post_shader_f = ""
	"uniform sampler2D fbo_texture;\n"
	"uniform float gamma;\n"
	"uniform float resx;\n"
	"uniform float resy;\n"
	"varying vec2 f_texcoord;\n"


	"void main(void) {\n"
	"vec3 c = texture2D(fbo_texture, f_texcoord).rgb;\n"
	"float lum = dot(c,vec3(0.299,0.587,0.114));\n"
	"vec2 uv = gl_FragCoord.xy / vec2(resx,resy);\n"
	
	"float xdist = 0.5 - uv.x;\n"
	"float ydist = 0.5 - uv.y;\n"
	"float m = 1.0 - sqrt(xdist*xdist+ydist*ydist);\n"
	"gl_FragColor = vec4(c,1.0);"
	"gl_FragColor.rgb = pow(gl_FragColor.rgb,vec3(gamma))*vec3(1.0-m/2.0,m,1.0-m/2.0);\n"
	"}\n";*/

#endif
