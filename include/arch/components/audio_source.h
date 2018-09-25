#include "defines.h"
#include "ECS/ecs_types.h"

#include "components/component_types.h"

#define MAX_AUDIO_WAIT_TIME 5.0

typedef struct play_audio_queue_data_t
{
	UUID name;
	real64 timer;
} PlayAudioQueueData;

void playSoundAtSource(
	AudioSourceComponent *audioSource,
	const char *soundName);
void pauseSoundAtSource(AudioSourceComponent *audioSource);
void resumeSoundAtSource(AudioSourceComponent *audioSource);
void stopSoundAtSource(AudioSourceComponent *audioSource);
bool isSourceActive(AudioSourceComponent *audioSource);

void pauseAllAudio(void);
void stopAllAudio(void);
void playAllAudio(void);
