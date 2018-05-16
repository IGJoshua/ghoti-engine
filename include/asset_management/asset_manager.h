#pragma once
#include "defines.h"

#include "renderer/renderer_types.h"

struct aiScene;
struct aiMesh;
struct aiString;

int32 loadScene(const char *name, Scene **scene);
int32 loadModel(char *name);
int32 loadMesh(const struct aiMesh *mesh, MeshData *meshData);
int32 loadMaterial(
	const struct aiScene *scene,
	const struct aiMesh *mesh,
	const MeshData *meshData,
	Material *material
);
int32 loadTexture(char *name, TextureType type);

Model* getModel(const char *name);
Texture* getTexture(const char *name);
uint32 getModelIndex(const char *name);
uint32 getTextureIndex(const char *name);

int32 freeModel(const char *name);
int32 freeMesh(Mesh *mesh);
int32 freeMaterial(Material *material);
int32 freeTexture(const char *name);
int32 unloadScene(Scene **scene);
