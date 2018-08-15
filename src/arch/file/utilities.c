#include "file/utilities.h"

#include "core/log.h"

#include <frozen/frozen.h>

#include <sys/stat.h>

#include <malloc.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#define MAX_JSON_LINE_LENGTH 80

internal char* readFile(const char *filename);
internal void writeCharacter(char character, uint32 n, FILE *file);
internal void formatJSON(const char *filename);
internal void formatJSONArrays(const char *filename);

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

int32 deleteFolder(const char *folder, bool errors)
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
					if (deleteFolder(folderPath, errors) == -1)
					{
						free(folderPath);
						closedir(dir);
						return -1;
					}
				}
				else if (S_ISREG(info.st_mode))
				{
					remove(folderPath);
				}

				free(folderPath);
			}

			dirEntry = readdir(dir);
		}

		closedir(dir);
	}
	else
	{
		if (errors)
		{
			LOG("Failed to open %s\n", folder);
		}
		return -1;
	}

	rmdir(folder);

	return 0;
}

void copyFile(const char *filename, const char *destination)
{
	char *fileBuffer = NULL;
	uint64 fileLength = 0;

	FILE *file = fopen(filename, "rb");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		fileLength = ftell(file);
		fseek(file, 0, SEEK_SET);
		fileBuffer = calloc(fileLength + 1, 1);

		if (fileBuffer)
		{
			fread(fileBuffer, 1, fileLength, file);
		}
	}

	if (fileBuffer)
	{
		file = freopen(destination, "wb", file);
		fwrite(fileBuffer, fileLength, 1, file);
		fclose(file);
		free(fileBuffer);
	}
}

int32 copyFolder(const char *folder, const char *destination)
{
	MKDIR(destination);

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
				char *destinationFolderPath = getFullFilePath(
					dirEntry->d_name,
					NULL,
					destination);

				struct stat info;
				stat(folderPath, &info);

				if (S_ISDIR(info.st_mode))
				{
					if (copyFolder(folderPath, destinationFolderPath) == -1)
					{
						free(folderPath);
						free(destinationFolderPath);
						closedir(dir);
						return -1;
					}
				}
				else if (S_ISREG(info.st_mode))
				{
					copyFile(folderPath, destinationFolderPath);
				}

				free(folderPath);
				free(destinationFolderPath);
			}

			dirEntry = readdir(dir);
		}

		closedir(dir);
	}
	else
	{
		LOG("Failed to open %s\n", folder);
		return -1;
	}

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

void writeJSON(const cJSON *json, const char *filename)
{
	char *jsonFilename = getFullFilePath(filename, "json", NULL);
	FILE *file = fopen(jsonFilename, "w");

	char *jsonString = cJSON_Print(json);
	fwrite(jsonString, strlen(jsonString), 1, file);
	free(jsonString);

	fclose(file);

	json_prettify_file(jsonFilename);
	formatJSON(jsonFilename);

	free(jsonFilename);
}

char* readFile(const char *filename)
{
	char *fileBuffer = NULL;
	uint64 fileLength = 0;

	FILE *file = fopen(filename, "r");

	if (file)
	{
		fseek(file, 0, SEEK_END);
		fileLength = ftell(file);
		fseek(file, 0, SEEK_SET);
		fileBuffer = calloc(fileLength + 1, 1);

		if (fileBuffer)
		{
			fread(fileBuffer, 1, fileLength, file);
		}

		fclose(file);
	}
	else
	{
		LOG("Unable to open %s\n", filename);
	}

	return fileBuffer;
}

void writeCharacter(char character, uint32 n, FILE *file)
{
	for (uint32 i = 0; i < n; i++)
	{
		fwrite(&character, sizeof(char), 1, file);
	}
}

void formatJSON(const char *filename)
{
	char *fileBuffer = readFile(filename);
	uint64 fileLength = strlen(fileBuffer);

	FILE *file = fopen(filename, "w");

	uint32 numSpaces;
	uint32 currentIndent;
	uint64 j;

	for (uint64 i = 0; i < fileLength; i++)
	{
		char character = fileBuffer[i];

		switch (character)
		{
			case '{':
			case '[':
				if (fileBuffer[i + 1] == '}')
				{
					fseek(file, -1, SEEK_CUR);
					writeCharacter('\n', 1, file);
					writeCharacter('\t', currentIndent, file);
					writeCharacter(character, 1, file);
					writeCharacter('\n', 1, file);
					writeCharacter('\t', currentIndent + 1, file);
					writeCharacter('\n', 1, file);
					writeCharacter('\t', currentIndent, file);
					character = fileBuffer[++i];
				}
				else if (i >= 2 &&
					fileBuffer[i - 2] == ':' &&
					fileBuffer[i + 1] != '}')
				{
					fseek(file, -1, SEEK_CUR);
					writeCharacter('\n', 1, file);
					writeCharacter('\t', currentIndent, file);
				}

				break;
			default:
				break;
		}

		writeCharacter(character, 1, file);

		switch (character) {
			case '\n':
				if (i - 1 > 0 &&
					fileBuffer[i - 1] == ',' &&
					i - 2 > 0 &&
					(fileBuffer[i - 2] == '}' || fileBuffer[i - 2] == ']'))
				{
					writeCharacter('\n', 1, file);
				}
				else
				{
					for (j = i + 1;
						j < fileLength && fileBuffer[j] != '\n';
						j++);

					if ((fileBuffer[--j] == '{' ||
						(fileBuffer[j - 1] == '{' && fileBuffer[j] == '}')) &&
						fileBuffer[i - 1] != '{' &&
						fileBuffer[i - 1] != '[')
					{
						writeCharacter('\n', 1, file);
					}
				}

				numSpaces = 0;
				for (j = i + 1; fileBuffer[j] == ' '; j++, i++)
				{
					numSpaces++;
				}

				currentIndent = numSpaces / 2;
				writeCharacter('\t', currentIndent, file);

				break;
			default:
				break;
		}
	}

	fclose(file);
	free(fileBuffer);

	formatJSONArrays(filename);
}

void formatJSONArrays(const char *filename)
{
	char *fileBuffer = readFile(filename);
	uint64 fileLength = strlen(fileBuffer);

	FILE *file = fopen(filename, "w");

	uint64 j;
	uint32 k;
	uint32 currentLineLength;
	char *array;
	uint64 arrayStart;
	uint32 arrayLength;
	uint32 formattedArrayLength;
	bool formatArray;
	char arrayCharacter;
	uint32 currentIndent;

	for (uint64 i = 0; i < fileLength; i++)
	{
		char character = fileBuffer[i];

		writeCharacter(character, 1, file);

		if (character == '\t')
		{
			currentLineLength += 4;
		}
		else
		{
			currentLineLength++;
		}

		switch (character)
		{
			case '\n':
				currentLineLength = 0;
				break;
			case ':':
				for (j = i + 1;
					fileBuffer[j] == '\n' || fileBuffer[j] == '\t';
					j++);

				if (fileBuffer[j] == '[')
				{
					arrayStart = j;

					formatArray = true;
					for (++j; fileBuffer[j] != ']'; j++)
					{
						if (fileBuffer[j] == '{' || fileBuffer[j] == '[')
						{
							formatArray = false;
							break;
						}
					}

					if (formatArray)
					{
						arrayLength = ++j - arrayStart;

						if (arrayLength > 2)
						{
							array = malloc(arrayLength);
							memcpy(array, &fileBuffer[arrayStart], arrayLength);

							formattedArrayLength = 0;
							for (j = 0; j < arrayLength; j++)
							{
								arrayCharacter = array[j];
								if (arrayCharacter != '\t')
								{
									formattedArrayLength++;
								}

								if (arrayCharacter == ',')
								{
									formattedArrayLength--;
								}
							}

							if (fileBuffer[arrayStart + arrayLength] == ',')
							{
								formattedArrayLength++;
							}

							if (++formattedArrayLength + currentLineLength
								<= MAX_JSON_LINE_LENGTH)
							{
								writeCharacter(' ', 1, file);

								for (j = 0; j < arrayLength; j++)
								{
									arrayCharacter = array[j];

									if (arrayCharacter == '\n')
									{
										if (array[j - 1] == '[')
										{
											continue;
										}

										for (k = j + 1; array[k] == '\t'; k++);
										if (array[k] == ']')
										{
											continue;
										}

										arrayCharacter = ' ';
									}
									else if (arrayCharacter == '\t')
									{
										continue;
									}

									writeCharacter(arrayCharacter, 1, file);
								}

								character = fileBuffer[
									i = arrayStart + arrayLength - 1];

								if (fileBuffer[i + 1] == ',')
								{
									writeCharacter(',', 1, file);

									for (k = 0, j = i + 2; k < 3; j++)
									{
										if (fileBuffer[j] == '\n')
										{
											k++;
										}
									}

									for (++j; fileBuffer[j] == '\t'; j++);

									i++;
									if (fileBuffer[j] != '{')
									{
										i++;
									}
								}
							}

							free(array);
						}
						else
						{
							for (currentIndent = 0, ++i; i <= arrayStart; i++)
							{
								character = fileBuffer[i];
								if (character == '\t')
								{
									currentIndent++;
								}

								writeCharacter(character, 1, file);
							}

							writeCharacter('\n', 1, file);
							writeCharacter('\t', currentIndent + 1, file);
							writeCharacter('\n', 1, file);
							writeCharacter('\t', currentIndent, file);

							i--;
						}
					}
				}

				break;
			default:
				break;
		}
	}

	fclose(file);
	free(fileBuffer);
}