#include "file/utilities.h"

#include "frozen/frozen.h"

#include <sys/stat.h>

#include <malloc.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

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

char* removeExtension(const char *filename)
{
	const char *extension = strrchr(filename, '.');

	char *name = malloc(extension - filename + 1);
	memcpy(name, filename, extension - filename);
	name[extension - filename] = '\0';

	return name;
}

void deleteFile(const char *filename, const char *folder)
{
	char *filepath = getFullFilePath(filename, NULL, folder);
	remove(filepath);
	free(filepath);
}

int32 deleteFolder(const char *folder)
{
	DIR *dir = opendir(folder);
	if (dir)
	{
		struct dirent *dirEntry = readdir(dir);
		while (dirEntry)
		{
			if (strcmp(dirEntry->d_name, ".") && strcmp(dirEntry->d_name, ".."))
			{
				char *folderPath = getFullFilePath(
					dirEntry->d_name,
					NULL,
					folder);

				struct stat info;
				stat(folderPath, &info);

				if (S_ISDIR(info.st_mode))
				{
					if (deleteFolder(folderPath) == -1)
					{
						free(folderPath);
						closedir(dir);
						return -1;
					}
				}
				else if (S_ISREG(info.st_mode))
				{
					deleteFile(dirEntry->d_name, folder);
				}

				free(folderPath);
			}

			dirEntry = readdir(dir);
		}
	}
	else
	{
		printf("Failed to open %s\n", folder);
		return -1;
	}

	closedir(dir);
	rmdir(folder);

	return 0;
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