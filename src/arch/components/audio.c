#include "defines.h"

#include "core/log.h"

#include "asset_management/audio.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"

#include "components/component_types.h"
#include "components/audio.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include <AL/alext.h>
#include <AL/efx-creative.h>
#include <AL/efx.h>
#include <AL/efx-presets.h>


extern ALCdevice *device;
extern ALCcontext *context;
extern ALCboolean g_bEAX;
extern ALuint *g_Buffers;
extern ALuint *g_Sources;
extern ALenum errorCode;

extern uint64 freeBuffer;
extern uint64 NUM_BUFF;
extern uint64 NUM_SRC;

bool system_setup;

void playSoundAtSource(Scene *scene, const char * soundName, UUID source)
{
    loadAudio(soundName);

    AudioFile *audioData;
    audioData = getAudio(soundName);

    if (!audioData)
    {
        LOG("%s.ogg was unable to be loaded", soundName);
        return;
    }

    UUID audioSourceComonentID = idFromName("audio_source");

    AudioSourceComponent *sourceComp = sceneGetComponentFromEntity(
        scene,
        source,
        audioSourceComonentID);

    if (!sourceComp)
    {
        LOG("%s does not have an audio source component\n", source.string);
        return;
    }

    alSourceRewind(g_Sources[sourceComp->id]);

    alSourcei(
        g_Sources[sourceComp->id],
        AL_BUFFER,
        g_Buffers[audioData->id]);

    alSourcePlay(g_Sources[sourceComp->id]);
    LOG("Playing %s.ogg at %s\n", soundName, source.string);
}

void queueSoundAtSource(Scene *scene, const char * soundName, UUID source)
{
    loadAudio(soundName);

    AudioFile *audioData;
    audioData = getAudio(soundName);

    if (!audioData)
    {
        LOG("%s.ogg was unable to be loaded\n", soundName);
        return;
    }

    UUID audioSourceComonentID = idFromName("audio_source");

    AudioSourceComponent *sourceComp = sceneGetComponentFromEntity(
        scene,
        source,
        audioSourceComonentID);

    if (!sourceComp)
    {
        LOG("%s does not have an audio source component\n", source.string);
        return;
    }

    alSourceQueueBuffers(
        g_Sources[sourceComp->id],
        1,
        &g_Buffers[audioData->id]);

    errorCode = alGetError();
    if (errorCode != AL_NO_ERROR)
    {
        LOG("OpenAL ERROR: unable to queue : %s\n", alGetString(errorCode));
        return;
    }

    LOG("Queued %s.ogg to %s\n", soundName, source.string);

}

void pauseSoundAtSource(Scene *scene, UUID source)
{
    UUID audioSourceComonentID = idFromName("audio_source");

    AudioSourceComponent *sourceComp = sceneGetComponentFromEntity(
        scene,
        source,
        audioSourceComonentID);

    if (!sourceComp)
    {
        LOG("%s does not have an audio source component\n", source.string);
        return;
    }

    alSourcePause(g_Sources[sourceComp->id]);
}

void resumeSoundAtSource(Scene *scene, UUID source)
{
    UUID audioSourceComonentID = idFromName("audio_source");

    AudioSourceComponent *sourceComp = sceneGetComponentFromEntity(
        scene,
        source,
        audioSourceComonentID);

    if (!sourceComp)
    {
        LOG("%s does not have an audio source component\n", source.string);
        return;
    }

    alSourcePlay(g_Sources[sourceComp->id]);
}

void stopSoundAtSource(Scene *scene, UUID source)
{
    UUID audioSourceComonentID = idFromName("audio_source");

    AudioSourceComponent *sourceComp = sceneGetComponentFromEntity(
        scene,
        source,
        audioSourceComonentID);

    if (!sourceComp)
    {
        LOG("%s does not have an audio source component\n", source.string);
        return;
    }

    alSourceStop(g_Sources[sourceComp->id]);
}

bool isSourceActive(Scene *scene, UUID source)
{
    UUID audioSourceComonentID = idFromName("audio_source");

    AudioSourceComponent *sourceComp = sceneGetComponentFromEntity(
        scene,
        source,
        audioSourceComonentID);

    if (!sourceComp)
    {
        LOG("%s does not have an audio source component\n", source.string);
        return false;
    }

    ALint state = 0;

    alGetSourcei(
        g_Sources[sourceComp->id],
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
