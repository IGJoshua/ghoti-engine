#include "asset_management/model.h"

#include "asset_management/material.h"
#include "asset_management/mesh.h"
#include "asset_management/texture.h"

#include "renderer/shader.h"

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <malloc.h>
#include <string.h>

extern Model *models;
extern uint32 numModels;
extern uint32 modelsCapacity;
extern Texture *textures;
extern uint32 numTextures;
extern uint32 texturesCapacity;

int32 loadModel(const char *name)
{
	if (numModels + 1 > modelsCapacity)
	{
		modelsCapacity += 16;
		uint32 previousBufferSize = numModels * sizeof(Model);
		uint32 newBufferSize = modelsCapacity * sizeof(Model);

		if (previousBufferSize == 0)
		{
			models = malloc(newBufferSize);
			memset(models, 0, newBufferSize);
		}
		else
		{
			models = realloc(models, newBufferSize);
			memset(
				models + previousBufferSize,
				0,
				newBufferSize - previousBufferSize
        	);
		}
	}

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

	if (numTextures + 2 > texturesCapacity)
	{
		// Assumes that all textures are unique, TODO Maybe don't assume this
		// to prevent allocation overhead?

		// TODO Check for unique textures in LoadMaterial() and
		// reallocate to a smaller size if some textures were unique

		// If texture is not unique, increase texture refcount
		uint32 previousBufferSize = numTextures * sizeof(Texture);
		// uint32 newBufferSize = (numTextures + scene->mNumTextures) * sizeof(Texture);
		uint32 newBufferSize = (numTextures + 2) * sizeof(Texture);

		if (previousBufferSize == 0)
		{
			textures = malloc(newBufferSize);
			memset(textures, 0, newBufferSize);
		}
		else
		{
			textures = realloc(textures, newBufferSize);

			memset(
				textures + previousBufferSize,
				0,
				newBufferSize - previousBufferSize
			);
		}
	}

	model->numMaterials = scene->mNumMaterials;
	model->materials = malloc(scene->mNumMaterials * sizeof(Material));
	memset(model->materials, 0, scene->mNumMaterials * sizeof(Material));

	for (uint32 i = 0; i < scene->mNumMaterials; i++)
	{
		loadMaterial(
			scene->mMaterials[i],
			&model->materials[i]
		);
	}

	model->numMeshes = scene->mNumMaterials;
	model->meshes = malloc(scene->mNumMaterials * sizeof(Mesh));

	for (uint32 i = 0; i < scene->mNumMaterials; i++)
	{
		MeshData meshData;
		memset(&meshData, 0, sizeof(MeshData));

		uint32 numVertices = 0;
		uint32 numIndices = 0;
		for (uint32 j = 0; j < scene->mNumMeshes; j++)
		{
			if (scene->mMeshes[j]->mMaterialIndex == i)
			{
				numVertices += scene->mMeshes[j]->mNumVertices;
				numIndices += scene->mMeshes[j]->mNumFaces * 3;
			}
		}

		meshData.colors = malloc(numVertices * sizeof(kmVec4));
		meshData.positions = malloc(numVertices * sizeof(kmVec3));
		meshData.normals = malloc(numVertices * sizeof(kmVec3));
		meshData.tangents = malloc(numVertices * sizeof(kmVec3));
		meshData.bitangents = malloc(numVertices * sizeof(kmVec3));
		meshData.uvs = malloc(numVertices * sizeof(kmVec2));
		meshData.indices = malloc(numIndices * sizeof(uint32));

		for (uint32 j = 0; j < scene->mNumMeshes; j++)
		{
			if (scene->mMeshes[j]->mMaterialIndex == i)
			{
				loadMesh(scene->mMeshes[j], &meshData);
			}
		}

		GLuint vao;
		glGenVertexArrays(1, &vao);

		model->meshes[i].vertexArray = vao;

		glBindVertexArray(vao);

		uint32 bufferIndex = 0;

		GLuint colorBuffer;
		glGenBuffers(1, &colorBuffer);

		model->meshes[i].colorBuffer = colorBuffer;

		glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(kmVec4) * numVertices, meshData.colors, GL_STATIC_DRAW);
		glVertexAttribPointer(bufferIndex++, 4, GL_FLOAT, GL_FALSE, 0, 0);

		GLuint positionBuffer;
		glGenBuffers(1, &positionBuffer);

		model->meshes[i].positionBuffer = positionBuffer;

		glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(kmVec3) * numVertices, meshData.positions, GL_STATIC_DRAW);
		glVertexAttribPointer(bufferIndex++, 3, GL_FLOAT, GL_FALSE, 0, 0);

		GLuint normalBuffer;
		glGenBuffers(1, &normalBuffer);

		model->meshes[i].normalBuffer = normalBuffer;

		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(kmVec3) * numVertices, meshData.normals, GL_STATIC_DRAW);
		glVertexAttribPointer(bufferIndex++, 3, GL_FLOAT, GL_TRUE, 0, 0);

		GLuint tangentBuffer;
		glGenBuffers(1, &tangentBuffer);

		model->meshes[i].tangentBuffer = tangentBuffer;

		glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(kmVec3) * numVertices, meshData.tangents, GL_STATIC_DRAW);
		glVertexAttribPointer(bufferIndex++, 3, GL_FLOAT, GL_TRUE, 0, 0);

		GLuint bitangentBuffer;
		glGenBuffers(1, &bitangentBuffer);

		model->meshes[i].bitangentBuffer = bitangentBuffer;

		glBindBuffer(GL_ARRAY_BUFFER, bitangentBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(kmVec3) * numVertices, meshData.bitangents, GL_STATIC_DRAW);
		glVertexAttribPointer(bufferIndex++, 3, GL_FLOAT, GL_TRUE, 0, 0);

		GLuint uvBuffer;
		glGenBuffers(1, &uvBuffer);

		model->meshes[i].uvBuffer = uvBuffer;

		glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(kmVec2) * numVertices, meshData.uvs, GL_STATIC_DRAW);
		glVertexAttribPointer(bufferIndex++, 2, GL_FLOAT, GL_FALSE, 0, 0);

		GLuint indexBuffer;
		glGenBuffers(1, &indexBuffer);

		model->meshes[i].indexBuffer = indexBuffer;

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * numIndices, meshData.indices, GL_STATIC_DRAW);

		model->meshes[i].numIndices = numIndices;

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glBindVertexArray(0);

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
	}

	aiReleaseImport(scene);

	model->refCount++;

	return 0;
}

Model* getModel(const char *name)
{
	if (name)
	{
		for (uint32 i = 0; i < numModels; i++)
		{
			if (strcmp(models[i].name, name) == 0)
			{
				return &models[i];
			}
		}
	}

	return NULL;
}

uint32 getModelIndex(const char *name)
{
	if (name)
	{
		for (uint32 i = 0; i < numModels; i++)
		{
			if (strcmp(models[i].name, name) == 0)
			{
				return i;
			}
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

		for (uint32 i = 0; i < model->numMeshes; i++)
		{
			freeMesh(&model->meshes[i]);
		}

		free(model->meshes);

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
				&resizedModels[index],
				&models[index + 1],
				(numModels - index) * sizeof(Model)
			);
		}

		free(models);
		models = resizedModels;
	}

	return 0;
}

// =================================
// Rendering

extern Shader vertShader;
extern Shader fragShader;

extern ShaderPipeline pipeline;

extern Uniform modelUniform;
extern Uniform viewUniform;
extern Uniform projectionUniform;

extern Uniform diffuseTextureUniform;

int32 renderModel(const char *name, kmMat4 *world, kmMat4 *view, kmMat4 *projection)
{
	Model *model = getModel(name);

	bindShaderPipeline(pipeline);

	setUniform(modelUniform, world);
	setUniform(viewUniform, view);
	setUniform(projectionUniform, projection);

	GLint textureIndex = 0;
	setUniform(diffuseTextureUniform, &textureIndex);

	for (uint32 i = 0; i < model->numMeshes; i++)
	{
		Mesh *mesh = &model->meshes[i];

		glBindVertexArray(mesh->vertexArray);

		for (uint32 i = 0; i < NUM_VERTEX_ATTRIBUTES; i++)
		{
			glEnableVertexAttribArray(i);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);

		Material *material = &model->materials[i];
		Texture *texture = getTexture(material->diffuseTexture);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture->id);

		glDrawElements(
			GL_TRIANGLES,
			mesh->numIndices,
			GL_UNSIGNED_INT,
			NULL
		);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		for (uint32 i = 0; i < NUM_VERTEX_ATTRIBUTES; i++)
		{
			glDisableVertexAttribArray(i);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	glUseProgram(0);

	return 0;
}
