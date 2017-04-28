#ifndef SOUND_H
#define SOUND_H

#include <SDL2/SDL_mixer.h> 
#include "comps.h"

// this is never not a pointer
typedef Mix_Chunk* Sound;

typedef struct sound_data
{
	int channels;
	int sound_count;
	#define MAX_SOUNDS 32
	Sound sounds[MAX_SOUNDS];

	int jump;
	int rifle;
	int grapple_shoot;
	int grapple_stick;
}sound_data;

sound_data *sound_data_create();
void sound_data_destroy(sound_data *data);
int sound_load(sound_data *data, char *file);
int sound_play(sound_data *data, int sound);
int sound_play_channel(sound_data *data, int sound, int channel);
void sound_play_at(sound_data *data, int sound, v3 listener, v3 position, float listener_dir);


#endif