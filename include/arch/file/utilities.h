#pragma once

#include "cJSON/cJSON.h"

#include <stdio.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(file, mode) _mkdir(file)
#else
#include <sys/stat.h>
#define MKDIR(file, mode) mkdir(file, mode)
#endif

char* getFolderPath(const char *filename, const char *parentFolder);
char* getFullFilename(
	const char *filename,
	const char *extension,
	const char *folder);
char* readString(FILE *file);
void writeString(const char *string, FILE *file);

void writeJSON(const cJSON *json, const char *filename);