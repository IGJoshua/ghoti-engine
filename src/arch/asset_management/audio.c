#include "asset_management/asset_manager_types.h"
#include "asset_management/audio.h"

#include "audio/audio.h"

#include "core/log.h"

#include "file/utilities.h"

#include "data/data_types.h"
#include "data/hash_map.h"

#include <AL/al.h>

#include <stb/stb_vorbis.c>

#include "ECS/scene.h"

extern HashMap audioFiles;
extern ALuint *g_Buffers;

internal ALenum errorCode = 0;
internal uint64 freeBuffer = 0;

int32 loadAudio(const char *name)
{
	AudioFile * existingAudio = getAudio(name);
	if(!existingAudio)
	{
		if (freeBuffer >= NUM_AUDIO_BUFF)
		{
			LOG("Unable to load %s.ogg. Too few buffers\n", name);
			return -1;
		}

		LOG("Loading audio file (%s.ogg)\n", name);

		AudioFile audioResource;
		audioResource.name = idFromName(name);

		char *filename = getFullFilePath(name, "ogg", "resources/audio");

		audioResource.size = stb_vorbis_decode_filename(
			filename,
			&audioResource.channels,
			&audioResource.sample_rate,
			&audioResource.output);

		free(filename);

		if(audioResource.size == -1)
		{
			LOG("Error while loading audio (%s.ogg) \n", name);
			return -1;
		}

		if(audioResource.channels == 1)
		{
			audioResource.format = AL_FORMAT_MONO16;
		}
		else
		{
			audioResource.format = AL_FORMAT_STEREO16;
		}

		uploadAudioToSoundCard(&audioResource);
		hashMapInsert(audioFiles, &audioResource.name, &audioResource);

		LOG("Successfully loaded audio (%s.ogg)\n", name);
	}

	return 0;
}

int32 uploadAudioToSoundCard(AudioFile *audio)
{
	LOG("Transferring audio (%s) onto sound card...\n", audio->name.string);

	alGetError();
	alBufferData(
		g_Buffers[freeBuffer],
		audio->format,
		audio->output,
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

AudioFile* getAudio(const char *name)
{
	AudioFile *sound = NULL;
	if (strlen(name) > 0)
	{
		UUID audioName = idFromName(name);
		sound = hashMapGetData(audioFiles, &audioName);
	}

	return sound;
}


void freeAudio(AudioFile * audio)
{
	if (audio)
	{
		UUID name = audio->name;

		LOG("Freeing audio (%s.ogg)...\n", name.string);

		free(audio->output);
		hashMapDelete(audioFiles, &name);

		LOG("Successfully freed audio (%s.ogg)\n", name.string);
	}
}
