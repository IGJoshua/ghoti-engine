#include "defines.h"

#include "core/log.h"

#include "asset_management/audio.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"

#include "components/audio.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/efx-creative.h>
#include <AL/efx.h>
#include <AL/efx-presets.h>


extern ALCdevice *device;
extern ALCcontext *context;
extern ALCboolean g_bEAX;
extern ALuint *g_Buffers;
ALuint *g_Sources = 0;
extern ALenum errorCode;

extern uint64 freeBuffer;
extern uint64 NUM_BUFF;
uint64 NUM_SRC = 32;

bool system_setup;

void playSoundAtSource(
	AudioSourceComponent *audioSource,
	const char *soundName)
{
    loadAudio(soundName);

    AudioFile *audioData;
    audioData = getAudio(soundName);

    if (!audioData)
    {
        LOG("%s.ogg was unable to be loaded\n", soundName);
        return;
    }

    alSourceRewind(g_Sources[audioSource->id]);

    alSourcei(
        g_Sources[audioSource->id],
        AL_BUFFER,
        g_Buffers[audioData->id]);

    alSourcePlay(g_Sources[audioSource->id]);
}

void queueSoundAtSource(
	AudioSourceComponent *audioSource,
	const char *soundName)
{
    loadAudio(soundName);

    AudioFile *audioData;
    audioData = getAudio(soundName);

    if (!audioData)
    {
        LOG("%s.ogg was unable to be loaded\n", soundName);
        return;
    }

    alSourceQueueBuffers(
        g_Sources[audioSource->id],
        1,
        &g_Buffers[audioData->id]);

    errorCode = alGetError();
    if (errorCode != AL_NO_ERROR)
    {
        LOG("OpenAL ERROR: unable to queue : %s\n", alGetString(errorCode));
        return;
    }
}

void pauseSoundAtSource(AudioSourceComponent *audioSource)
{
    alSourcePause(g_Sources[audioSource->id]);
}

void resumeSoundAtSource(AudioSourceComponent *audioSource)
{
    alSourcePlay(g_Sources[audioSource->id]);
}

void stopSoundAtSource(AudioSourceComponent *audioSource)
{
    alSourceStop(g_Sources[audioSource->id]);
}

bool isSourceActive(AudioSourceComponent *audioSource)
{
    ALint state = 0;

    alGetSourcei(
        g_Sources[audioSource->id],
        AL_SOURCE_STATE,
        &state);

    if (state == AL_PLAYING || state == AL_PAUSED)
    {
        return true;
    }

    return false;
}

void pauseAllAudio(void)
{
    alSourcePausev(NUM_SRC, g_Sources);
}

void stopAllAudio(void)
{
    alSourceStopv(NUM_SRC, g_Sources);
}

void playAllAudio(void)
{
    alSourcePlayv(NUM_SRC, g_Sources);
}
