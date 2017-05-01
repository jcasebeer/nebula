#ifndef SOUND_H
#define SOUND_H

//#include <SDL2/SDL_mixer.h> 
#include <AL/al.h>
#include <AL/alut.h>
#include "comps.h"


typedef struct sound_data
{
	#define MAX_SOURCES 32
	int sources[MAX_SOURCES];
	int jump;
	int rifle;
	int grapple_shoot;
	int grapple_stick;
}sound_data;

sound_data *sound_data_create();
void sound_data_destroy(sound_data *data);
int sound_load(char *file);
void sound_play(sound_data *data, int sound);
void sound_play_at(sound_data *data, int sound, v3 position);
void sound_listener_set(v3 pos, v3 vel, float dir, float zdir);


#endif