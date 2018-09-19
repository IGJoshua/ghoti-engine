#include "defines.h"
#include "ECS/ecs_types.h"

#include "components/component_types.h"

void playSoundAtSource(
	AudioSourceComponent *audioSource,
	const char *soundName);
void queueSoundAtSource(
	AudioSourceComponent *audioSource,
	const char *soundName);
void pauseSoundAtSource(AudioSourceComponent *audioSource);
void resumeSoundAtSource(AudioSourceComponent *audioSource);
void stopSoundAtSource(AudioSourceComponent *audioSource);
bool isSourceActive(AudioSourceComponent *audioSource);

void pauseAllAudio(void);
void stopAllAudio(void);
void playAllAudio(void);
