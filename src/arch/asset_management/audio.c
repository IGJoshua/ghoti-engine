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

        alGetError();
        alBufferData(
            g_Buffers[freeBuffer],
            audioResource.format,
            audioResource.output,
            audioResource.size * sizeof(int16) * audioResource.channels,
            audioResource.sample_rate);

        errorCode = alGetError();

        if (errorCode != AL_NO_ERROR)
        {
            LOG("alBufferData buffer 0 : %s\n", alGetString(errorCode));
            alDeleteBuffers(1, &g_Buffers[freeBuffer]);
            return -1;
        }

        audioResource.id = freeBuffer;
        freeBuffer++;

        hashMapInsert(audioFiles, &audioResource.name, &audioResource);

        LOG("Successfully loaded audio (%s.ogg)\n", name);
    }

    return 0;
}

AudioFile* getAudio(const char *name)
{
    AudioFile *sound = NULL;
	if (strlen(name) > 0)
	{
		UUID nameID = idFromName(name);
		sound = hashMapGetData(audioFiles, &nameID);
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
