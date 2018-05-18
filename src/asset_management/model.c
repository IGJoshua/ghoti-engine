#include "asset_management/model.h"
#include "asset_management/mesh.h"
#include "asset_management/material.h"

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <malloc.h>
#include <string.h>

extern Model *models;
extern uint32 numModels;
extern Texture *textures;
extern uint32 numTextures;

int32 loadModel(const char *name)
{
	Model *model = &models[numModels++];

	model->name = malloc(256);
	strcpy(model->name, name);
	
	char filename[1024];
	sprintf(filename, "resources/models/%s.dae", name);
	const struct aiScene *scene = aiImportFile(
		filename,
		aiProcessPreset_TargetRealtime_Quality &
		~aiProcess_SplitLargeMeshes
	);

	// Assumes that all textures are unique, TODO Maybe don't assume this
	// to prevent allocation overhead?
	
	// TODO Check for unique textures in LoadMaterial() and
	// reallocate to a smaller size if some textures were unique
	
	// If texture is not unique, increase texture refcount
	uint32 previousBufferSize = numTextures * sizeof(Texture);
	// uint32 newBufferSize = (numTextures + scene->mNumTextures) * sizeof(Texture);
	uint32 newBufferSize = (numTextures + 1) * sizeof(Texture);
	
	if (previousBufferSize == 0)
	{
		textures = malloc(newBufferSize);
#ifdef _DEBUG
		memset(textures, 0, newBufferSize);
#endif
	}
	else
	{
		textures = realloc(textures, newBufferSize);

#ifdef _DEBUG
		memset(
			textures + previousBufferSize,
			0,
			newBufferSize - previousBufferSize
		);
#endif
	}

	MeshData meshData;
	memset(&meshData, 0, sizeof(MeshData));
	
	uint32 numVertices = 0;
	uint32 numIndices = 0;
	for (uint32 i = 0; i < scene->mNumMeshes; i++)
	{
		numVertices += scene->mMeshes[i]->mNumVertices;
		numIndices += scene->mMeshes[i]->mNumFaces * 3;
	}

	meshData.colors = malloc(numVertices * sizeof(kmVec4));
	meshData.positions = malloc(numVertices * sizeof(kmVec3));
	meshData.normals = malloc(numVertices * sizeof(kmVec3));
	meshData.tangents = malloc(numVertices * sizeof(kmVec3));
	meshData.bitangents = malloc(numVertices * sizeof(kmVec3));
	meshData.uvs = malloc(numVertices * sizeof(kmVec2));
	meshData.indices = malloc(numIndices * sizeof(uint32));

	model->numMaterials = scene->mNumMeshes;
	model->materials = malloc(scene->mNumMeshes * sizeof(Material));
	memset(model->materials, 0, scene->mNumMeshes * sizeof(Material));

	for (uint32 i = 0; i < scene->mNumMeshes; i++)
	{
		loadMesh(scene->mMeshes[i], &meshData);
		loadMaterial(
			scene,
			scene->mMeshes[i],
			&meshData,
			&model->materials[i]
		);
	}
	
	GLuint vao;
	glGenVertexArrays(1, &vao);

	model->mesh.vertexArray = vao;

	glBindVertexArray(vao);

	uint32 bufferIndex = 0;
	
	GLuint colorBuffer;
	glGenBuffers(1, &colorBuffer);

	model->mesh.colorBuffer = colorBuffer;

	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(kmVec4) * numVertices, meshData.colors, GL_STATIC_DRAW);
	glVertexAttribPointer(bufferIndex++, 4, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint positionBuffer;
	glGenBuffers(1, &positionBuffer);

	model->mesh.positionBuffer = positionBuffer;

	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(kmVec3) * numVertices, meshData.positions, GL_STATIC_DRAW);
	glVertexAttribPointer(bufferIndex++, 3, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint normalBuffer;
	glGenBuffers(1, &normalBuffer);

	model->mesh.normalBuffer = normalBuffer;

	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(kmVec3) * numVertices, meshData.normals, GL_STATIC_DRAW);
	glVertexAttribPointer(bufferIndex++, 3, GL_FLOAT, GL_TRUE, 0, 0);
	
	GLuint tangentBuffer;
	glGenBuffers(1, &tangentBuffer);

	model->mesh.tangentBuffer = tangentBuffer;

	glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(kmVec3) * numVertices, meshData.tangents, GL_STATIC_DRAW);
	glVertexAttribPointer(bufferIndex++, 3, GL_FLOAT, GL_TRUE, 0, 0);
	
	GLuint bitangentBuffer;
	glGenBuffers(1, &bitangentBuffer);

	model->mesh.bitangentBuffer = bitangentBuffer;

	glBindBuffer(GL_ARRAY_BUFFER, bitangentBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(kmVec3) * numVertices, meshData.bitangents, GL_STATIC_DRAW);
	glVertexAttribPointer(bufferIndex++, 3, GL_FLOAT, GL_TRUE, 0, 0);

	GLuint uvBuffer;
	glGenBuffers(1, &uvBuffer);

	model->mesh.uvBuffer = uvBuffer;

	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(kmVec2) * numVertices, meshData.uvs, GL_STATIC_DRAW);
	glVertexAttribPointer(bufferIndex++, 2, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint indexBuffer;
	glGenBuffers(1, &indexBuffer);

	model->mesh.indexBuffer = indexBuffer;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * numIndices, meshData.indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	aiReleaseImport(scene);

	free(meshData.colors);
	meshData.colors = 0;
	free(meshData.positions);
	meshData.positions = 0;
	free(meshData.normals);
	meshData.normals = 0;
	free(meshData.tangents);
	meshData.normals = 0;
	free(meshData.bitangents);
	meshData.normals = 0;
	free(meshData.uvs);
	meshData.uvs = 0;
	free(meshData.indices);
	meshData.indices = 0;

	model->refCount++;
	
	return 0;
}

Model* getModel(const char *name)
{
	for (uint32 i = 0; i < numModels; i++)
	{
		if (strcmp(models[i].name, name) == 0)
		{
			return &models[i];
		}
	}

	return NULL;
}

uint32 getModelIndex(const char *name)
{
	for (uint32 i = 0; i < numModels; i++)
	{
		if (strcmp(models[i].name, name) == 0)
		{
			return i;
		}
	}

	return -1;
}

int32 freeModel(const char *name)
{
	Model *model = getModel(name);
	uint32 index = getModelIndex(name);

	if (--(model->refCount) == 0)
	{
		free(model->name);
		freeMesh(&model->mesh);

		for (uint32 i = 0; i < model->numMaterials; i++)
		{
			freeMaterial(&model->materials[i]);
		}

		free(model->materials);

		Model *resizedModels = malloc(--numModels * sizeof(Model));
		memcpy(resizedModels, models, index * sizeof(Model));

		if (index < numModels)
		{
			memcpy(
				resizedModels + index * sizeof(Model),
				model + sizeof(Model),
				(numModels - index) * sizeof(Model)
			);
		}

		free(models);
		models = resizedModels;
	}

	return 0;
}
