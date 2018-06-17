#include "ECS/save.h"
#include "ECS/scene.h"

#include "data/hash_map.h"

#include "file/utilities.h"

#include "json-utilities/utilities.h"

#include <sys/stat.h>

#include <malloc.h>
#include <dirent.h>

void exportSave(void *data, uint32 size, const Scene *scene, uint32 slot)
{
	char *saveName = malloc(128);
	sprintf(saveName, "save_%d", slot);

	printf("Exporting save file (%s)...\n", saveName);

	char *saveFolder = malloc(512);
	sprintf(saveFolder, "resources/saves/%s", saveName);
	char *saveFilename = getFullFilePath(saveName, "save", saveFolder);

	MKDIR(saveFolder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	char *sceneFolder = getFolderPath(scene->name, saveFolder);
	char *sceneFilename = getFullFilePath(scene->name, NULL, sceneFolder);
	char *entitiesFolder = getFullFilePath("entities", NULL, sceneFolder);

	MKDIR(sceneFolder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	MKDIR(entitiesFolder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	FILE *file = fopen(saveFilename, "wb");

	writeString(scene->name, file);
	fwrite(&size, sizeof(uint32), 1, file);
	fwrite(data, size, 1, file);

	fclose(file);

	exportSceneSnapshot(scene, sceneFilename);

	exportScene(sceneFilename);
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

		exportEntity(entityFilename);
		char *jsonEntityFilename = getFullFilePath(
			entityFilename,
			"json",
			NULL);
		remove(jsonEntityFilename);

		free(jsonEntityFilename);
		free(entityFilename);
	}

	free(saveFolder);
	free(saveFilename);
	free(sceneFolder);
	free(sceneFilename);
	free(entitiesFolder);

	printf("Successfully exported save file (%s)\n", saveName);
	free(saveName);
}

int32 loadSave(uint32 slot, Scene **scene)
{
	int32 error = 0;

	char *saveName = malloc(128);
	sprintf(saveName, "save_%d", slot);

	printf("Loading save file (%s)...\n", saveName);

	char *saveFolder = malloc(512);
	sprintf(saveFolder, "resources/saves/%s", saveName);
	char *saveFilename = getFullFilePath(saveName, "save", saveFolder);

	FILE *file = fopen(saveFilename, "rb");

	if (file)
	{
		char *sceneName = readString(file);
		char *sceneFolder = getFolderPath(sceneName, saveFolder);

		loadScene(sceneName, sceneFolder, scene);

		free(sceneFolder);
		free(sceneName);

		uint32 size;
		fread(&size, sizeof(uint32), 1, file);

		// TODO: Pass global data from save to Lua
		// void *data;
		// fread(&size, size, 1, file);
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
	return -1;
}

bool getSaveSlotAvailability(uint32 slot)
{
	bool available = false;

	char *saveName = malloc(128);
	sprintf(saveName, "save_%d", slot);

	char *saveFolder = malloc(512);
	sprintf(saveFolder, "resources/saves/%s", saveName);
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
		char *saveFolder = malloc(512);
		sprintf(saveFolder, "resources/saves/%s", saveName);

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