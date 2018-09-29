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

extern HashMap audioFiles;
extern pthread_mutex_t audioMutex;

extern HashMap loadingAudio;
extern pthread_mutex_t loadingAudioMutex;

extern HashMap uploadAudioQueue;
extern pthread_mutex_t uploadAudioMutex;

extern uint32 assetThreadCount;
extern pthread_mutex_t assetThreadsMutex;
extern pthread_cond_t assetThreadsCondition;

extern HashMap audioFiles;
extern ALuint *g_Buffers;

internal ALenum errorCode = 0;
internal uint64 freeBuffer = 0;

internal void* acquireAudioThread(void *arg);
internal void* loadAudioThread(void *arg);

void loadAudio(const char *name)
{
	char *audioName = calloc(1, strlen(name) + 1);
	strcpy(audioName, name);

	pthread_t acquisitionThread;
	pthread_create(
		&acquisitionThread,
		NULL,
		&acquireAudioThread,
		(void*)audioName);
	pthread_detach(acquisitionThread);
}

void* acquireAudioThread(void *arg)
{
	pthread_mutex_lock(&assetThreadsMutex);

	while (assetThreadCount == config.assetsConfig.maxThreadCount)
	{
		pthread_cond_wait(&assetThreadsCondition, &assetThreadsMutex);
	}

	assetThreadCount++;

	pthread_mutex_unlock(&assetThreadsMutex);
	pthread_cond_broadcast(&assetThreadsCondition);

	pthread_t loadingThread;
	pthread_create(&loadingThread, NULL, &loadAudioThread, arg);
	pthread_detach(loadingThread);

	EXIT_THREAD(NULL);
}

void* loadAudioThread(void *arg)
{
	int32 error = 0;

	char *name = arg;

	UUID audioName = idFromName(name);

	pthread_mutex_lock(&audioMutex);
	AudioFile *audioResource = hashMapGetData(audioFiles, &audioName);

	if (!audioResource)
	{
		pthread_mutex_unlock(&audioMutex);
		pthread_mutex_lock(&loadingAudioMutex);

		if (hashMapGetData(loadingAudio, &audioName))
		{
			error = 1;
		}

		pthread_mutex_unlock(&loadingAudioMutex);

		if (error != 1)
		{
			pthread_mutex_lock(&uploadAudioMutex);

			if (hashMapGetData(uploadAudioQueue, &audioName))
			{
				error = 1;
			}

			pthread_mutex_unlock(&uploadAudioMutex);
		}

		if (error != 1)
		{
			bool loading = true;
			pthread_mutex_lock(&loadingAudioMutex);
			hashMapInsert(loadingAudio, &audioName, &loading);
			pthread_mutex_unlock(&loadingAudioMutex);

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
				audio.lifetime = config.assetsConfig.minAudioLifetime;

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
		}
	}
	else
	{
		pthread_mutex_unlock(&audioMutex);
	}

	free(name);

	pthread_mutex_lock(&assetThreadsMutex);
	assetThreadCount--;
	pthread_mutex_unlock(&assetThreadsMutex);
	pthread_cond_broadcast(&assetThreadsCondition);

	EXIT_THREAD(NULL);
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

AudioFile getAudio(const char *name)
{
	AudioFile audio = {};

	if (strlen(name) > 0)
	{
		UUID audioName = idFromName(name);

		pthread_mutex_lock(&audioMutex);

		AudioFile *audioResource = hashMapGetData(audioFiles, &audioName);
		if (audioResource)
		{
			audioResource->lifetime = config.assetsConfig.minAudioLifetime;
			audio = *audioResource;
		}

		pthread_mutex_unlock(&audioMutex);
	}

	return audio;
}

void freeAudioData(AudioFile *audio)
{
	LOG("Freeing audio (%s)...\n", audio->name.string);

	free(audio->data);

	LOG("Successfully freed audio (%s)\n", audio->name.string);
}