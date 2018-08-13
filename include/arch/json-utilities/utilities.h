#pragma once
#include "defines.h"

int32 generateEntity(const char *filename, const char *logFilename);
int32 exportEntity(const char *filename, const char *logFilename);
int32 exportAsset(const char *filename, const char *logFilename);
int32 generateScene(const char *filename, const char *logFilename);
int32 exportScene(const char *filename, const char *logFilename);

UUID generateUUID(void);