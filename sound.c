#include <stdio.h>
#include <stdlib.h>
#include "sound.h"
#include "comps.h"
#include "gmath.h"

sound_data *sound_data_create()
{
	sound_data *data = malloc(sizeof(sound_data));
	//printf("ALUT Initialization: %d\n",alutInit(NULL,NULL));
	data->device = alcOpenDevice(NULL); // open default device
	data->context = alcCreateContext(data->device,NULL);
	alcMakeContextCurrent(data->context);
//*** 
	//for using alutCreateBufferFromContext()
	alutInitWithoutContext(NULL,NULL);
	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
	alListenerf(AL_GAIN,0.9f);
	alGenSources(MAX_SOURCES,(ALuint *)data->sources);
	for(int i = 0; i<MAX_SOURCES; i++)
	{
		alSourcef(data->sources[i],AL_ROLLOFF_FACTOR,1.f);
		alSourcef(data->sources[i],AL_MAX_DISTANCE,2048.f);
		alSourcef(data->sources[i],AL_REFERENCE_DISTANCE,512.f);
	}
	return data;
}

void sound_data_destroy(sound_data *data)
{
	alDeleteSources(MAX_SOURCES,(ALuint *)data->sources);
	data->device = alcGetContextsDevice(data->context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(data->context);
	alcCloseDevice(data->device);
	free(data);
}

int sound_load(char *file)
{
	int buffer = 0;

	buffer = (int) alutCreateBufferFromFile(file);
	return buffer;
//***
}

void sound_play(sound_data *data,int sound)
{
	// find source
	int source = sound_get_source(data);

	if (source == -1)
		return;

	alSourcei(source,AL_BUFFER,(ALuint) sound);
	float lpos[3];
	alGetListenerfv(AL_POSITION,(ALfloat *)lpos);
	alSourcefv(source,AL_POSITION,lpos);
	alSourcePlay(source); 
}

int sound_get_source(sound_data *data)
{
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
	return source;
}

void sound_play_loop(sound_data *data, int sound)
{
	// find buffer where sound is already playing
	int source = -1;
	for(int i = 0; i<MAX_SOURCES; i++)
	{
		int buffer,state;
		alGetSourcei(data->sources[i],AL_BUFFER,&buffer);
		alGetSourcei(data->sources[i],AL_SOURCE_STATE,&state);
		if (buffer == sound && state==AL_PLAYING)
		{
			source = data->sources[i];
			break;
		}
	}
	// if the sound is not playing, play it
	if (source == -1)
		sound_play(data,sound);

}

void sound_free(int sound)
{
	alDeleteBuffers(1,(const unsigned int *)&sound);
}


void sound_play_at(sound_data *data, int sound, v3 position)
{
	// find source
	int source = sound_get_source(data);

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

	orientation[0] = lengthdir_x(lengthdir_x(1,-zdir),dir);
	orientation[1] = lengthdir_y(lengthdir_x(1,-zdir),dir);
	orientation[2] = lengthdir_x(1,-zdir);

	//float zdir2 = zdir+90.f;
	orientation[3] = 0.f;//lengthdir_x(lengthdir_x(1,-zdir2),dir);
	orientation[4] = 0.f;//lengthdir_y(lengthdir_x(1,-zdir2),dir);
	orientation[5] = 1.f;//lengthdir_x(1,-zdir2);
	alListenerfv(AL_ORIENTATION,orientation);
}