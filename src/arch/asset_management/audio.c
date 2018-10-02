#include "asset_management/asset_manager.h"
#include "asset_management/audio.h"

#include "audio/audio.h"

#include "core/log.h"
#include "core/config.h"

#include "data/data_types.h"
#include "data/hash_map.h"

#include "ECS/scene.h"

#include "file/utilities.h"

#include <AL/al.h>

#include <stb/stb_vorbis.c>

#include <pthread.h>

extern Config config;

EXTERN_ASSET_VARIABLES(audioFiles, Audio);
EXTERN_ASSET_MANAGER_VARIABLES;

extern HashMap audioFiles;
extern ALuint *g_Buffers;

internal ALenum errorCode = 0;
internal uint64 freeBuffer = 0;

INTERNAL_ASSET_THREAD_VARIABLES(Audio);

void loadAudio(const char *name)
{
	char *audioName = calloc(1, strlen(name) + 1);
	strcpy(audioName, name);

	bool skip = false;

	UUID nameID = idFromName(name);

	pthread_mutex_lock(&audioFilesMutex);
	AudioFile *audioResource = hashMapGetData(audioFiles, &nameID);

	if (!audioResource)
	{
		pthread_mutex_unlock(&audioFilesMutex);
		pthread_mutex_lock(&loadingAudioMutex);

		if (hashMapGetData(loadingAudio, &nameID))
		{
			skip = true;
		}

		pthread_mutex_unlock(&loadingAudioMutex);

		if (!skip)
		{
			pthread_mutex_lock(&uploadAudioMutex);

			if (hashMapGetData(uploadAudioQueue, &nameID))
			{
				skip = true;
			}

			pthread_mutex_unlock(&uploadAudioMutex);
		}

		if (!skip)
		{
			START_ACQUISITION_THREAD(audio, Audio, Audio, audioName, nameID);
		}
	}
	else
	{
		pthread_mutex_unlock(&audioFilesMutex);
	}
}

ACQUISITION_THREAD(Audio);

void* loadAudioThread(void *arg)
{
	int32 error = 0;

	char *name = arg;

	UUID audioName = idFromName(name);

	if (freeBuffer >= NUM_AUDIO_BUFF)
	{
		ASSET_LOG(
			AUDIO,
			name,
			"Unable to load %s.ogg. Too few buffers\n",
			name);
		error = -1;
	}
	else
	{
		ASSET_LOG(AUDIO, name, "Loading audio file (%s.ogg)\n", name);

		AudioFile audio = {};

		audio.name = audioName;
		audio.lifetime = config.assetsConfig.minAudioFileLifetime;

		char *filename = getFullFilePath(
			name,
			"ogg",
			"resources/audio");

		audio.size = stb_vorbis_decode_filename(
			filename,
			&audio.channels,
			&audio.sample_rate,
			&audio.data);

		free(filename);

		if(audio.size == -1)
		{
			ASSET_LOG(
				AUDIO,
				name,
				"Failed to load audio file (%s.ogg) \n",
				name);
			error = -1;
		}

		if(audio.channels == 1)
		{
			audio.format = AL_FORMAT_MONO16;
		}
		else
		{
			audio.format = AL_FORMAT_STEREO16;
		}

		if (error != -1)
		{
			pthread_mutex_lock(&uploadAudioMutex);
			hashMapInsert(uploadAudioQueue, &audioName, &audio);
			pthread_mutex_unlock(&uploadAudioMutex);

			ASSET_LOG(
				AUDIO,
				name,
				"Successfully loaded audio file (%s.ogg)\n",
				name);
		}
	}

	ASSET_LOG_COMMIT(AUDIO, name);

	pthread_mutex_lock(&loadingAudioMutex);
	hashMapDelete(loadingAudio, &audioName);
	pthread_mutex_unlock(&loadingAudioMutex);

	free(name);

	EXIT_LOADING_THREAD;
}

int32 uploadAudioToSoundCard(AudioFile *audio)
{
	LOG("Transferring audio (%s) onto sound card...\n", audio->name.string);

	alGetError();
	alBufferData(
		g_Buffers[freeBuffer],
		audio->format,
		audio->data,
		audio->size * sizeof(int16) * audio->channels,
		audio->sample_rate);

	errorCode = alGetError();

	if (errorCode != AL_NO_ERROR)
	{
		LOG("alBufferData buffer %d: %s\n",
			(int32)freeBuffer,
			alGetString(errorCode));
		alDeleteBuffers(1, &g_Buffers[freeBuffer]);
	}
	else
	{
		audio->id = freeBuffer++;

		LOG("Successfully transferred audio (%s) onto sound card\n",
			audio->name.string);
	}

	return errorCode;
}

GET_ASSET_FUNCTION(
	audio,
	audioFiles,
	AudioFile,
	getAudio(const char *name),
	idFromName(name));

void freeAudioFileData(AudioFile *audio)
{
	LOG("Freeing audio (%s)...\n", audio->name.string);

	free(audio->data);

	LOG("Successfully freed audio (%s)\n", audio->name.string);
}