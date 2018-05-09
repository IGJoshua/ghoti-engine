#pragma once
#include "defines.h"

#include "renderer/renderer_types.h"

struct aiScene;
struct aiMesh;

int32 loadScene(
	const char *name,
	Model *models,
	uint32 *numModels,
	Texture *textures,
	uint32 *numTextures,
	Scene **scene
);

int32 loadModel(
	char *name,
	Texture *textures,
	uint32 *numTextures,
	Model *model
);

int32 loadMesh(
	const struct aiMesh *mesh,
	MeshData *meshData
);

int32 loadMaterial(
	const struct aiScene *scene,
	const struct aiMesh *mesh,
	const MeshData *meshData,
	Texture *textures,
	uint32 *numTextures,
	Material *material
);

int32 loadTexture(
	char *name,
	TextureType type,
	Texture *texture
);

bool isUniqueModel(
	const char *name,
	Model *models,
	uint32 *numModels
);

bool isUniqueTexture(
	const char *name,
	Texture *textures,
	uint32 *numTextures
);

int32 freeModel(
	const char *name,
	Model *models,
	uint32 *numModels
);

int32 freeTexture(
	const char *name,
	Texture *textures,
	uint32 *numTextures
);

int32 unloadScene(Scene **scene);
