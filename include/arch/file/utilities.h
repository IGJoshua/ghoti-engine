#pragma once

#include "cJSON/cJSON.h"

#include <stdio.h>

char* getFolderPath(const char *filename, const char *parentFolder);
char* getFullFilename(
	const char *filename,
	const char *extension,
	const char *folder);
char* readString(FILE *file);

void writeJSON(const cJSON *json, const char *filename);