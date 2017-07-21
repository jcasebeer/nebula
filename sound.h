#ifndef SOUND_H
#define SOUND_H

//#include <SDL2/SDL_mixer.h> 
#include <AL/al.h>
#include <AL/alc.h>
#include <sndfile.h>
#include "comps.h"

typedef struct sound_data
{
	ALCdevice *device;
	ALCcontext *context;
	#define MAX_SOURCES 32
	unsigned int sources[MAX_SOURCES];
	unsigned int jump;
	unsigned int rifle;
	unsigned int grapple_shoot;
	unsigned int grapple_stick;
	unsigned int grapple_buzz;
	unsigned int grapple_end;
}sound_data;

extern sound_data *SOUND;

sound_data *sound_data_create();
void sound_data_destroy(sound_data *data);
int sound_load(char *file);
void sound_play(sound_data *data, unsigned int sound);
void sound_play_at(sound_data *data, unsigned int sound, v3 position);
void sound_listener_set(v3 pos, v3 vel, float dir, float zdir);
void sound_free(sound_data *data, unsigned int sound);
void sound_play_loop(sound_data *data, unsigned int sound);
int sound_get_source(sound_data *data);

#endif