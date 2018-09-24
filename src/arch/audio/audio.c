#include "defines.h"

#include "audio/audio.h"

#include "components/audio_source.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/list.h"

#include "ECS/ecs_types.h"

#include <AL/al.h>
#include <AL/alc.h>

extern Scene* listenerScene;
extern List activeScenes;

internal ALCdevice *device = NULL;
internal ALCcontext *context = NULL;
internal ALCboolean g_bEAX = false;

ALuint *g_Buffers = NULL;
ALuint *g_Sources = NULL;

int32 initAudio(void)
{
	ALenum errorCode = 0;

	device = alcOpenDevice(NULL);

	if (device)
	{
		context = alcCreateContext(device, NULL);

		if (context)
		{
			alcMakeContextCurrent(context);
		}
		else
		{
			LOG("Could not create audio context\n");
			return -1;
		}
	}
	else
	{
		LOG("Could not create audio device\n");

		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);

		return -1;
	}

	//Check for EAX 2.0
	g_bEAX = alIsExtensionPresent("EAX2.0");

	//Create Buffers
	alGetError();

	g_Buffers = malloc(sizeof(ALuint)*NUM_AUDIO_BUFF);
	alGenBuffers(NUM_AUDIO_BUFF, g_Buffers);

	errorCode = alGetError();
	if (errorCode != AL_NO_ERROR)
	{
		LOG("alGenBuffers has errored: %s\n", alGetString(errorCode));

		free(g_Buffers);

		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);
		alcCloseDevice(device);

		return -1;
	}

	// Generate Sources
	g_Sources = malloc(sizeof(ALuint)*NUM_AUDIO_SRC);
	alGenSources(NUM_AUDIO_SRC, g_Sources);

	errorCode = alGetError();
	if (errorCode != AL_NO_ERROR)
	{
		LOG("alGenSources has errored : %s\n", alGetString(errorCode));

		alDeleteBuffers(NUM_AUDIO_BUFF, g_Buffers);

		free(g_Buffers);
		free(g_Sources);

		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);
		alcCloseDevice(device);

		return -1;
	}

	return 0;
}

void setListenerScene(const char *name)
{
    for (ListIterator itr = listGetIterator(&activeScenes);
         !listIteratorAtEnd(itr);
         listMoveIterator(&itr))
    {
        Scene *scene = *LIST_ITERATOR_GET_ELEMENT(Scene*, itr);
        if (!strcmp(name, scene->name))
        {
            listenerScene = scene;
            return;
        }
    }
}

void shutdownAudio(void)
{
	stopAllAudio();

	alDeleteBuffers(NUM_AUDIO_BUFF, g_Buffers);
	free(g_Buffers);

	alDeleteSources(NUM_AUDIO_SRC, g_Sources);
	free(g_Sources);

	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
}