#include "file/utilities.h"

#include "defines.h"

#include <malloc.h>
#include <string.h>

char* getFolderPath(const char *filename, const char *parentFolder)
{
	char *folderPath = malloc(strlen(parentFolder) + strlen(filename) + 2);
	sprintf(folderPath, "%s/%s", parentFolder, filename);
	return folderPath;
}

char* getFullFilename(
	const char *filename,
	const char *extension,
	const char *folder)
{
	char *fullFilename = malloc(
		strlen(folder) +
		strlen(filename) +
		strlen(extension) +
		3);
	sprintf(fullFilename, "%s/%s.%s", folder, filename, extension);
	return fullFilename;
}

char* readString(FILE *file)
{
	uint32 stringLength;
	fread(&stringLength, sizeof(uint32), 1, file);

	char *string = malloc(stringLength);
	fread(string, stringLength, 1, file);

	return string;
}