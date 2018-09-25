#include "defines.h"

#include "asset_management/audio.h"

#include "audio/audio.h"

#include "components/audio_source.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"

#include <AL/al.h>

extern ALuint *g_Buffers;
extern ALuint *g_Sources;

void playSoundAtSource(
	AudioSourceComponent *audioSource,
	const char *soundName)
{
	loadAudio(soundName);

	AudioFile audioData = getAudio(soundName);
	if (strlen(audioData.name.string) == 0)
	{
		LOG("%s.ogg was unable to be loaded\n", soundName);
		return;
	}

	alSourceRewind(g_Sources[audioSource->id]);

	alSourcei(
		g_Sources[audioSource->id],
		AL_BUFFER,
		g_Buffers[audioData.id]);

	alSourcePlay(g_Sources[audioSource->id]);
}

void queueSoundAtSource(
	AudioSourceComponent *audioSource,
	const char *soundName)
{
	// loadAudio(soundName);

	// AudioFile audioData = getAudio(soundName);
	// if (strlen(audioData.name.string) == 0)
	// 	LOG("%s.ogg was unable to be loaded\n", soundName);
	// 	return;
	// }

	// alSourceQueueBuffers(
	// 	g_Sources[audioSource->id],
	// 	1,
	// 	&g_Buffers[audioData.id]);

	// ALenum errorCode = alGetError();
	// if (errorCode != AL_NO_ERROR)
	// {
	// 	LOG("OpenAL ERROR: unable to queue : %s\n", alGetString(errorCode));
	// 	return;
	// }
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
	alSourcePausev(NUM_AUDIO_SRC, g_Sources);
}

void stopAllAudio(void)
{
	alSourceStopv(NUM_AUDIO_SRC, g_Sources);
}

void playAllAudio(void)
{
	alSourcePlayv(NUM_AUDIO_SRC, g_Sources);
}
