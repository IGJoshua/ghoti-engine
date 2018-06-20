#include "ECS/save.h"
#include "ECS/scene.h"

#include "data/hash_map.h"
#include "data/list.h"

#include "file/utilities.h"

#include "json-utilities/utilities.h"

#include <sys/stat.h>

#include <malloc.h>
#include <string.h>
#include <dirent.h>

extern List activeScenes;

int32 exportSave(void *data, uint32 size, uint32 slot)
{
	char *saveName = malloc(128);
	sprintf(saveName, "save_%d", slot);

	printf("Exporting save file (%s)...\n", saveName);

	char *saveFolder = getFullFilePath(saveName, NULL, "resources/saves");
	MKDIR(saveFolder);

	char *saveFilename = getFullFilePath(saveName, "save", saveFolder);
	FILE *file = fopen(saveFilename, "wb");
	free(saveFilename);

	uint32 numActiveScenes = listGetSize(&activeScenes);
	fwrite(&numActiveScenes, sizeof(uint32), 1, file);

	for (ListIterator itr = listGetIterator(&activeScenes);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		Scene *scene = *LIST_ITERATOR_GET_ELEMENT(Scene*, itr);
		writeString(scene->name, file);
	}

	fwrite(&size, sizeof(uint32), 1, file);
	fwrite(data, size, 1, file);

	fclose(file);

	DIR *dir = opendir(RUNTIME_STATE_DIR);
	if (dir)
	{
		struct dirent *dirEntry = readdir(dir);
		while (dirEntry)
		{
			if (strcmp(dirEntry->d_name, ".")
			&& strcmp(dirEntry->d_name, ".."))
			{
				char *folderPath = getFullFilePath(
					dirEntry->d_name,
					NULL,
					RUNTIME_STATE_DIR);

				struct stat info;
				stat(folderPath, &info);

				if (S_ISDIR(info.st_mode))
				{
					char *destinationFolderPath = getFullFilePath(
						dirEntry->d_name,
						NULL,
						saveFolder);

					if (copyFolder(folderPath, destinationFolderPath) == -1)
					{
						free(folderPath);
						free(destinationFolderPath);
						closedir(dir);
						return -1;
					}

					free(folderPath);
					free(destinationFolderPath);
				}
			}

			dirEntry = readdir(dir);
		}

		closedir(dir);
	}

	for (ListIterator itr = listGetIterator(&activeScenes);
		 !listIteratorAtEnd(itr);
		 listMoveIterator(&itr))
	{
		Scene *scene = *LIST_ITERATOR_GET_ELEMENT(Scene*, itr);

		char *sceneFolder = getFullFilePath(scene->name, NULL, saveFolder);
		char *sceneFilename = getFullFilePath(scene->name, NULL, sceneFolder);
		char *entitiesFolder = getFullFilePath("entities", NULL, sceneFolder);

		MKDIR(sceneFolder);
		MKDIR(entitiesFolder);

		exportSceneSnapshot(scene, sceneFilename);

		if (exportScene(sceneFilename) == -1)
		{
			free(saveFolder);
			free(sceneFolder);
			free(sceneFilename);
			free(entitiesFolder);
			return -1;
		}

		char *jsonSceneFilename = getFullFilePath(
			sceneFilename,
			"json",
			NULL);
		remove(jsonSceneFilename);
		free(jsonSceneFilename);

		uint32 entityNumber = 0;
		for (HashMapIterator itr = hashMapGetIterator(scene->entities);
			!hashMapIteratorAtEnd(itr);
			hashMapMoveIterator(&itr))
		{
			char *entityName = malloc(128);
			sprintf(entityName, "entity_%d", entityNumber++);
			char *entityFilename = getFullFilePath(
				entityName,
				NULL,
				entitiesFolder);
			free(entityName);

			UUID *entity = (UUID*)hashMapIteratorGetKey(itr);
			exportEntitySnapshot(scene, *entity, entityFilename);

			if (exportEntity(entityFilename) == -1)
			{
				free(saveFolder);
				free(sceneFolder);
				free(sceneFilename);
				free(entitiesFolder);
				free(entityFilename);
				return -1;
			}

			char *jsonEntityFilename = getFullFilePath(
				entityFilename,
				"json",
				NULL);
			remove(jsonEntityFilename);

			free(jsonEntityFilename);
			free(entityFilename);
		}

		free(sceneFolder);
		free(sceneFilename);
		free(entitiesFolder);
	}

	free(saveFolder);

	printf("Successfully exported save file (%s)\n", saveName);
	free(saveName);

	return 0;
}

int32 loadSave(uint32 slot, void **data)
{
	int32 error = 0;

	char *saveName = malloc(128);
	sprintf(saveName, "save_%d", slot);

	printf("Loading save file (%s)...\n", saveName);

	char *saveFolder = getFullFilePath(saveName, NULL, "resources/saves");
	char *saveFilename = getFullFilePath(saveName, "save", saveFolder);

	FILE *file = fopen(saveFilename, "rb");

	if (file)
	{
		uint32 numActiveScenes;
		fread(&numActiveScenes, sizeof(uint32), 1, file);

		for (uint32 i = 0; i < numActiveScenes; i++)
		{
			char *sceneName = readString(file);
			char *sceneFolder = getFullFilePath(sceneName, NULL, saveFolder);

			MKDIR(RUNTIME_STATE_DIR);
			deleteFolder(RUNTIME_STATE_DIR);
			MKDIR(RUNTIME_STATE_DIR);

			DIR *dir = opendir(saveFolder);
			if (dir)
			{
				struct dirent *dirEntry = readdir(dir);
				while (dirEntry)
				{
					if (strcmp(dirEntry->d_name, ".")
					&& strcmp(dirEntry->d_name, ".."))
					{
						char *folderPath = getFullFilePath(
							dirEntry->d_name,
							NULL,
							saveFolder);

						struct stat info;
						stat(folderPath, &info);

						if (S_ISDIR(info.st_mode))
						{
							char *destinationFolderPath = getFullFilePath(
								dirEntry->d_name,
								NULL,
								RUNTIME_STATE_DIR);

							if (copyFolder(
								folderPath,
								destinationFolderPath) == -1)
							{
								error = -1;
							}

							free(destinationFolderPath);

							if (error == -1)
							{
								free(folderPath);
								break;
							}
						}

						free(folderPath);
					}

					dirEntry = readdir(dir);
				}

				closedir(dir);
			}
			else
			{
				printf("Failed to open %s\n", saveFolder);
				error = -1;
			}

			if (error != -1)
			{
				loadScene(sceneName);
			}

			free(sceneFolder);
			free(sceneName);
		}

		uint32 size;
		fread(&size, sizeof(uint32), 1, file);

		if (data)
		{
			fread(*data, size, 1, file);
		}
	}
	else
	{
		printf("Failed to open %s\n", saveFilename);
		error = -1;
	}

	free(saveFolder);
	free(saveFilename);

	if (error != -1)
	{
		printf("Successfully loaded save file (%s)\n", saveName);
	}

	free(saveName);
	return 0;
}

bool getSaveSlotAvailability(uint32 slot)
{
	bool available = false;

	char *saveName = malloc(128);
	sprintf(saveName, "save_%d", slot);

	char *saveFolder = getFullFilePath(saveName, NULL, "resources/saves");
	free(saveName);

    struct stat info;
	stat(saveFolder, &info);
	if (S_ISDIR(info.st_mode))
	{
		available = true;
	}

	free(saveFolder);
	return available;
}

int32 deleteSave(uint32 slot)
{
	int32 error = 0;

	char *saveName = malloc(128);
	sprintf(saveName, "save_%d", slot);

	printf("Deleting save file (%s)...\n", saveName);

	if (!getSaveSlotAvailability(slot))
	{
		printf("Save file doesn't exist.\n");
		error = -1;
	}

	if (error != -1)
	{
		char *saveFolder = getFullFilePath(saveName, NULL, "resources/saves");

		error = deleteFolder(saveFolder);
		free(saveFolder);

		if (error != -1)
		{
			printf("Successfully deleted save file (%s)\n", saveName);
		}
	}

	free(saveName);

	return error;
}