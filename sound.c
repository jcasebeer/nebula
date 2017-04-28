#include <stdio.h>
#include <stdlib.h>
#include "sound.h"
#include "comps.h"
#include "gmath.h"
#include <SDL2/SDL_mixer.h>

sound_data *sound_data_create()
{
	sound_data *data = malloc(sizeof(sound_data));
	
	data->sound_count = 0;
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT,2 ,4096) <0)
	{
		printf("Audio Error: %s\n",Mix_GetError());
		return NULL; 
	}
	data->channels = Mix_AllocateChannels(256);
	printf("Audio Mix Channels Allocated: %d\n", data->channels);
	return data;
}

void sound_data_destroy(sound_data *data)
{
	// free all of our sounds
	for(int i = 0; i<data->sound_count; i++)
		Mix_FreeChunk(data->sounds[i]);
	free(data);
	Mix_CloseAudio();
}

int sound_load(sound_data *data, char *file)
{
	data->sounds[data->sound_count] = Mix_LoadWAV(file);
	data->sound_count++;
	return data->sound_count-1;
}

int sound_play(sound_data *data, int sound)
{
	int channel = Mix_PlayChannel(-1, data->sounds[sound], 0);
	Mix_SetPosition(channel,0,0);
	return channel;
}

int sound_play_channel(sound_data *data, int sound, int channel)
{
	Mix_PlayChannel(channel, data->sounds[sound], 0);
	Mix_SetPosition(channel,0,0);
	return channel;
}


void sound_play_at(sound_data *data, int sound, v3 listener, v3 position, float listener_dir)
{
	// calculate magnitude and angle
	float diffx, diffy, diffz;
	diffx = position.x - listener.x;
	diffy = position.y - listener.y;
	diffz = position.z - listener.z;

	float distance = sqrt(diffx*diffx + diffy*diffy + diffz*diffz);
	unsigned char idist;
	float angle = 0.f;
	
	angle = fmod(RAD2DEG*(atan2(diffy,diffx))+180.f,360.f) - fmod(listener_dir,360.f);
	angle = fmod(angle,360.f);
	idist = (clamp(distance,0,1024.f)/1024.f)*255;
	
	// post process channel
	if (distance < 1024)
	{
		int channel = sound_play_channel(data,sound,-1);
		Mix_SetPosition(channel,(int)angle,idist);
	}
	
}
