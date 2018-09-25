#pragma once
#include "defines.h"

#include "asset_management/asset_manager_types.h"

void loadAudio(const char *name);
int32 uploadAudioToSoundCard(AudioFile *audio);
AudioFile getAudio(const char *name);
void freeAudioData(AudioFile *audio);