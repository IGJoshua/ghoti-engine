#pragma once

#include "defines.h"

#include <stdio.h>

#include <cjson/cJSON.h>

#define MIN_UUID_ASCII '#'
#define MAX_UUID_ASCII 'z'

#define MAX_JSON_LINE_LENGTH 80

#ifdef _WIN32
#include <direct.h>
#define MKDIR(folder) _mkdir(folder)
#else
#include <sys/stat.h>
#define MKDIR(folder) mkdir(folder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif

typedef void (*Log)(const char *format, ...);

UUID generateUUID(void);
UUID stringToUUID(const char *string);

char* concatenateStrings(
	const char *stringA,
	const char *separator,
	const char *stringB);
char* getFullFilePath(
	const char *filename,
	const char *extension,
	const char *folder);
char* getPrefix(const char *string, const char *delimiter);
char* getSuffix(const char *string, const char *delimiter);

char* getParentFolder(const char *filePath);
char* getFilename(const char *filePath);
char* getExtension(const char *filename);
char* removeExtension(const char *filename);

void writeString(const char *string, FILE *file);
void writeUUID(UUID uuid, FILE *file);
void writeStringAsUUID(const char *string, FILE *file);
void writeCharacter(char character, uint32 n, FILE *file);

char* readString(FILE *file);
UUID readUUID(FILE *file);
UUID readStringAsUUID(FILE *file);
char* readFile(const char *filename, uint64 *fileLength, Log log);

void copyFile(const char *filename, const char *destination, Log log);
void renameFile(
	const char *filename,
	const char *extension,
	const char *folder,
	const char *newFilename,
	const char *newExtension,
	Log log);

int32 copyFolder(const char *folder, const char *destination, Log log);
int32 deleteFolder(const char *folder, bool errors, Log log);

cJSON* loadJSON(const char *filename, Log log);
uint32 getJSONListSize(const cJSON *list);
void writeJSON(
	const cJSON *json,
	const char *filename,
	bool formatted,
	Log log);