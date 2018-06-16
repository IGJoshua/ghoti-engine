#include "file/utilities.h"

#include "defines.h"

#include "frozen/frozen.h"

#include <malloc.h>
#include <string.h>

char* getFolderPath(const char *filename, const char *parentFolder)
{
	char *folderPath = malloc(strlen(parentFolder) + strlen(filename) + 2);
	sprintf(folderPath, "%s/%s", parentFolder, filename);
	return folderPath;
}

char* getFullFilePath(
	const char *filename,
	const char *extension,
	const char *folder)
{
	uint32 numExtraCharacters = 1;

	if (extension)
	{
		numExtraCharacters += strlen(extension) + 1;
	}

	if (folder)
	{
		numExtraCharacters += strlen(folder) + 1;
	}

	char *fullFilename = malloc(strlen(filename) + numExtraCharacters);

	if (extension && folder)
	{
		sprintf(fullFilename, "%s/%s.%s", folder, filename, extension);
	}
	else if (extension)
	{
		sprintf(fullFilename, "%s.%s", filename, extension);
	}
	else if (folder)
	{
		sprintf(fullFilename, "%s/%s", folder, filename);
	}
	else
	{
		sprintf(fullFilename, "%s", filename);
	}

	return fullFilename;
}

char* getExtension(const char *filename)
{
	const char *a = strrchr(filename, '.');
	if (a)
	{
		const char *b = filename + strlen(filename);

		char *extension = malloc(b - a);
		memcpy(extension, a + 1, b - a);
		return extension;
	}

	return NULL;
}

char* readString(FILE *file)
{
	uint32 stringLength;
	fread(&stringLength, sizeof(uint32), 1, file);

	char *string = malloc(stringLength);
	fread(string, stringLength, 1, file);

	return string;
}

void writeString(const char *string, FILE *file)
{
	uint32 stringLength = strlen(string) + 1;
	fwrite(&stringLength, sizeof(uint32), 1, file);
	fwrite(string, stringLength, 1, file);
}

void writeJSON(const cJSON *json, const char *filename) {
	char *jsonFilename = getFullFilePath(filename, "json", NULL);
	FILE *file = fopen(jsonFilename, "w");

	char *jsonString = cJSON_Print(json);
	fwrite(jsonString, strlen(jsonString), 1, file);
	free(jsonString);

	fclose(file);

	json_prettify_file(jsonFilename);
	free(jsonFilename);
}