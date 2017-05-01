#include <stdio.h>
#include <stdlib.h>
#include "sound.h"
#include "comps.h"
#include "gmath.h"

sound_data *sound_data_create()
{
	sound_data *data = malloc(sizeof(sound_data));
	printf("ALUT Initialization: %d\n",alutInit(NULL,NULL));
	alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
	alListenerf(AL_GAIN,0.5f);
	alGenSources(MAX_SOURCES,(ALuint *)data->sources);
	for(int i = 0; i<MAX_SOURCES; i++)
	{
		alSourcef(data->sources[i],AL_ROLLOFF_FACTOR,1.f);
		alSourcef(data->sources[i],AL_MAX_DISTANCE,1024.f);
		alSourcef(data->sources[i],AL_REFERENCE_DISTANCE,512.f);
	}
	return data;
}

void sound_data_destroy(sound_data *data)
{
	alDeleteSources(MAX_SOURCES,(ALuint *)data->sources);
	alutExit();
	free(data);
}

int sound_load(char *file)
{
	int buffer;
	buffer = (int) alutCreateBufferFromFile(file);
	return buffer;
}

void sound_play(sound_data *data,int sound)
{
	// find source
	int source = -1;
	for(int i = 0; i<MAX_SOURCES; i++)
	{
		int state;
		alGetSourcei(data->sources[i],AL_SOURCE_STATE,&state);
		if (state != AL_PLAYING)
		{
			source = data->sources[i];
			break;
		}
	}

	alSourcei(source,AL_BUFFER,(ALuint) sound);
	float lpos[3];
	alGetListenerfv(AL_POSITION,(ALfloat *)lpos);
	alSourcefv(source,AL_POSITION,lpos);
	alSourcePlay(source); 
}



void sound_play_at(sound_data *data, int sound, v3 position)
{
	// find source
	int source = -1;
	for(int i = 0; i<MAX_SOURCES; i++)
	{
		int state;
		alGetSourcei(data->sources[i],AL_SOURCE_STATE,&state);
		if (state != AL_PLAYING)
		{
			source = data->sources[i];
			break;
		}
	}

	if (source == -1)
		return;

	alSourcei(source,AL_BUFFER,(ALuint) sound);
	alSource3f(source,AL_POSITION,position.x,position.y,position.z);
	alSourcePlay(source);

}

void sound_listener_set(v3 pos, v3 vel, float dir, float zdir)
{
	alListener3f(AL_POSITION,pos.x,pos.y,pos.z);
	alListener3f(AL_VELOCITY,vel.x,vel.y,vel.z);
	float orientation[6];

	orientation[1] = lengthdir_x(lengthdir_x(1,-zdir),dir);
	orientation[2] = lengthdir_y(lengthdir_x(1,-zdir),dir);
	orientation[3] = lengthdir_x(1,-zdir);

	orientation[3] = 0.f;
	orientation[4] = 0.f;
	orientation[5] = 1.f;
	alListenerfv(AL_ORIENTATION,orientation);
}