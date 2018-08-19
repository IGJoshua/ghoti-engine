#pragma once
#include "defines.h"

int32 exportEntity(const char *filename, const char *logFilename);
int32 exportScene(const char *filename, const char *logFilename);

UUID generateUUID(void);