#include "defines.h"
#include "ECS/ecs_types.h"

void playSoundAtSource(Scene *scene, const char *soundName, UUID source);
void queueSoundAtSource(Scene *scene, const char *soundName, UUID source);
void pauseSoundAtSource(Scene *scene, UUID source);
void resumeSoundAtSource(Scene *scene, UUID source);
void stopSoundAtSource(Scene *scene, UUID source);
bool isSourceActive(Scene *scene, UUID source);

void pauseAllAudio(void);
void stopAllAudio(void);
void playAllAudio(void);
