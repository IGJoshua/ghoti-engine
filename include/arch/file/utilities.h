#pragma once
#include "defines.h"

#include "cJSON/cJSON.h"

#include <stdio.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(file) _mkdir(file)
#else
#include <sys/stat.h>
#define MKDIR(file) mkdir(file, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif

char* getFullFilePath(
	const char *filename,
	const char *extension,
	const char *folder);
char* getExtension(const char *filename);
char* removeExtension(const char *filename);
int32 deleteFolder(const char *folder, bool errors);
void copyFile(const char *filename, const char *destination);
int32 copyFolder(const char *folder, const char *destination);
char* readString(FILE *file);
void writeString(const char *string, FILE *file);

void writeJSON(const cJSON *json, const char *filename);