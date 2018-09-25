#pragma once
#include "defines.h"

#define NUM_AUDIO_BUFF 32
#define NUM_AUDIO_SRC 32

int32 initAudio(void);
void setListenerScene(const char *name);
void shutdownAudio(void);