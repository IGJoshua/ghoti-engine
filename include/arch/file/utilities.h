#pragma once
#include "defines.h"

#include "components/component_types.h"

#include <cjson/cJSON.h>

#include <stdio.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(folder) _mkdir(folder)
#else
#include <sys/stat.h>
#define MKDIR(folder) mkdir(folder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
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

char* readFile(const char *filename, uint64 *fileLength);
char* readString(FILE *file);
UUID readStringAsUUID(FILE *file);
UUID readUUID(FILE *file);
TransformComponent readTransform(FILE *file);
void writeString(const char *string, FILE *file);

cJSON* loadJSON(const char *filename);
void writeJSON(const cJSON *json, const char *filename, bool formatted);