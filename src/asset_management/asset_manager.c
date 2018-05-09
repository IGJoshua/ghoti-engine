#include "asset_management/asset_manager.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <kazmath/vec4.h>
#include <kazmath/vec3.h>
#include <kazmath/vec2.h>

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/material.h>

#include <IL/il.h>
#include <IL/ilu.h>

// TODO Add JSON loading
// #include <cjson/cJSON.h>

#include <malloc.h>
#include <string.h>

int32 loadScene(
	const char *name,
	Model *models,
	uint32 *numModels,
	Texture *textures,
	uint32 *numTextures,
	Scene **scene)
{
	// TODO Get number of models in the scene from JSON file,
	// check for only models that haven't already been loaded
	// If model has already been loaded, increase refcount
	uint32 numSceneModels = 1;
	
	char **modelNames = malloc(numSceneModels * sizeof(char*));
	for (uint32 i = 0; i < numSceneModels; i++)
	{
		modelNames[i] = malloc(256);
		// TODO Get unique model names from JSON file
		modelNames[i] = "teapot";
	}

	if (scene)
	{
		*scene = malloc(sizeof(Scene));
#ifdef _DEBUG
		memset(*scene, 0, sizeof(Scene));
#endif
		(*scene)->models = modelNames;
	}

	uint32 previousBufferSize = *numModels * sizeof(Model);
	uint32 newBufferSize = (*numModels + numSceneModels) * sizeof(Model);
	
	if (previousBufferSize == 0)
	{
		models = malloc(newBufferSize);
#ifdef _DEBUG
		memset(models, 0, newBufferSize);
#endif
	}
	else
	{
		models = realloc(models, newBufferSize);

#ifdef _DEBUG
		memset(
			models + previousBufferSize,
			0,
			newBufferSize - previousBufferSize
		);
#endif
	}
	
	for (uint32 i = 0; i < numSceneModels; i++)
	{
		loadModel(modelNames[i], textures, numTextures, &models[(*numModels)++]);
		
		if (!scene)
		{
			free(modelNames[i]);
		}
	}
	
	if (!scene)
	{
		free(modelNames);
	}
	
	return 0;
}

int32 loadModel(
	char *name,
	Texture *textures,
	uint32 *numTextures,
	Model *model)
{
	model->name = name;
	
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
	uint32 previousBufferSize = *numTextures * sizeof(Texture);
	uint32 newBufferSize = (*numTextures + scene->mNumTextures) * sizeof(Texture);
	
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

	// TODO Have unique materials with lists of subsetOffsetPairs?
	model->numMaterials = scene->mNumMeshes;
	model->materials = malloc(scene->mNumMeshes * sizeof(Material));

	for (uint32 i = 0; i < scene->mNumMeshes; i++)
	{
		loadMesh(scene->mMeshes[i], &meshData);
		loadMaterial(
			scene,
			scene->mMeshes[i],
			&meshData,
			textures,
			numTextures,
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

int32 loadMesh(
	const struct aiMesh *mesh,
	MeshData *meshData)
{
	// TODO: Change this to use the stride and offset
	// to allow the entire thing to be stored in one array
	for (uint32 i = 0; i < mesh->mNumVertices; ++i)
	{
		uint32 index = meshData->numVertices;
		meshData->colors[index].x = 0.0f;
		meshData->colors[index].y = 0.0f;
		meshData->colors[index].z = 0.0f;
		meshData->colors[index].w = 1.0f;

		meshData->positions[index].x = mesh->mVertices[i].x;
		meshData->positions[index].y = mesh->mVertices[i].y;
		meshData->positions[index].z = mesh->mVertices[i].z;

		meshData->normals[index].x = mesh->mNormals[i].x;
		meshData->normals[index].y = mesh->mNormals[i].y;
		meshData->normals[index].z = mesh->mNormals[i].z;
		
		meshData->tangents[index].x = mesh->mTangents[i].x;
		meshData->tangents[index].y = mesh->mTangents[i].y;
		meshData->tangents[index].z = mesh->mTangents[i].z;
		
		meshData->bitangents[index].x = mesh->mBitangents[i].x;
		meshData->bitangents[index].y = mesh->mBitangents[i].y;
		meshData->bitangents[index].z = mesh->mBitangents[i].z;

		meshData->uvs[index].x = mesh->mTextureCoords[0][i].x;
		meshData->uvs[index].y = mesh->mTextureCoords[0][i].y;
		
		meshData->numVertices++;
	}

	uint32 numIndices = mesh->mNumFaces * 3;
	for (uint32 i = 0; i < mesh->mNumFaces; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			meshData->indices[meshData->numIndices++] = mesh->mFaces[i].mIndices[j];
		}
	}

	return 0;
}

int32 loadMaterial(
	const struct aiScene *scene,
	const struct aiMesh *mesh,
	const MeshData *meshData,
	Texture *textures,
	uint32 *numTextures,
	Material *material)
{
	material->type = MATERIAL_TYPE_DEBUG;

	const struct aiMaterial *materialData = scene->mMaterials[mesh->mMaterialIndex];

	// TODO Error Checking using if-else statements

	struct aiString textureName;

	if (aiGetMaterialString(
		materialData,
		AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0),
		&textureName) == AI_SUCCESS)
	{
		material->diffuseTexture = textureName.data;
		loadTexture(textureName.data, TEXTURE_TYPE_DIFFUSE, &textures[(*numTextures)++]);
	}

	if (aiGetMaterialString(
		materialData,
		AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0),
		&textureName) == AI_SUCCESS)
	{
		material->specularTexture = textureName.data;
		loadTexture(textureName.data, TEXTURE_TYPE_SPECULAR, &textures[(*numTextures)++]);
	}

	if (aiGetMaterialString(
		materialData,
		AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0),
		&textureName) == AI_SUCCESS)
	{
		material->normalMap = textureName.data;
		loadTexture(textureName.data, TEXTURE_TYPE_NORMAL, &textures[(*numTextures)++]);
	}

	if (aiGetMaterialString(
		materialData,
		AI_MATKEY_TEXTURE(aiTextureType_EMISSIVE, 0),
		&textureName) == AI_SUCCESS)
	{
		material->emissiveMap = textureName.data;
		loadTexture(textureName.data, TEXTURE_TYPE_EMISSIVE, &textures[(*numTextures)++]);
	}

	struct aiColor4D materialValue;

	if (aiGetMaterialColor(
		materialData,
		AI_MATKEY_COLOR_DIFFUSE,
		&materialValue) == AI_SUCCESS)
	{
		material->diffuseValue.x = materialValue.r;
		material->diffuseValue.y = materialValue.g;
		material->diffuseValue.z = materialValue.b;
	}

	if (aiGetMaterialColor(
		materialData,
		AI_MATKEY_COLOR_SPECULAR,
		&materialValue) == AI_SUCCESS)
	{
		material->specularValue.x = materialValue.r;
		material->specularValue.y = materialValue.g;
		material->specularValue.z = materialValue.b;
	}

	if (aiGetMaterialColor(
		materialData,
		AI_MATKEY_COLOR_AMBIENT,
		&materialValue) == AI_SUCCESS)
	{
		material->ambientValue.x = materialValue.r;
		material->ambientValue.y = materialValue.g;
		material->ambientValue.z = materialValue.b;
	}

	if (aiGetMaterialColor(
		materialData,
		AI_MATKEY_COLOR_EMISSIVE,
		&materialValue) == AI_SUCCESS)
	{
		material->emissiveValue.x = materialValue.r;
		material->emissiveValue.y = materialValue.g;
		material->emissiveValue.z = materialValue.b;
	}

	if (aiGetMaterialColor(
		materialData,
		AI_MATKEY_COLOR_TRANSPARENT,
		&materialValue) == AI_SUCCESS)
	{
		material->transparentValue.x = materialValue.r;
		material->transparentValue.y = materialValue.g;
		material->transparentValue.z = materialValue.b;
	}

	real32 materialConstant;

	if (aiGetMaterialFloatArray(
		materialData,
		AI_MATKEY_SHININESS,
		&materialConstant,
		NULL) == AI_SUCCESS)
	{
		material->specularPower = materialConstant;
	}

	if (aiGetMaterialFloatArray(
		materialData,
		AI_MATKEY_SHININESS_STRENGTH,
		&materialConstant,
		NULL) == AI_SUCCESS)
	{
		material->specularScale = materialConstant;
	}

	if (aiGetMaterialFloatArray(
		materialData,
		AI_MATKEY_OPACITY,
		&materialConstant,
		NULL) == AI_SUCCESS)
	{
		material->opacity = materialConstant;
	}

	material->subsetOffset = meshData->numIndices;

	return 0;
}

int32 loadTexture(
	char *name,
	TextureType type,
	Texture *texture)
{
	texture->name = name;
	texture->type = type;

	ILuint devilID;
	ilGenImages(1, &devilID);
	ilBindImage(devilID);
	
	char filename[1024];
	sprintf(filename, "resources/textures/%s", name);
	ilLoadImage(filename);

	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

	GLuint textureID;
	glGenTextures(1, &textureID);

	texture->id = textureID;

	glBindTexture(GL_TEXTURE_2D, textureID);

	GLsizei textureWidth = ilGetInteger(IL_IMAGE_WIDTH);
	GLsizei textureHeight = ilGetInteger(IL_IMAGE_HEIGHT);
	const GLvoid *textureData = ilGetData();
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA8,
		textureWidth,
		textureHeight,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		textureData
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	ilDeleteImages(1, &devilID);

	texture->refCount++;

	return 0;
}

bool isUniqueModel(
	const char *name,
	Model *models,
	uint32 *numModels)
{
	for (uint32 i = 0; i < *numModels; i++)
	{
		if (strcmp(models[i].name, name) == 0)
		{
			return false;
		}
	}

	return true;
}

bool isUniqueTexture(
	const char *name,
	Texture *textures,
	uint32 *numTextures)
{
	for (uint32 i = 0; i < *numTextures; i++)
	{
		if (strcmp(textures[i].name, name) == 0)
		{
			return false;
		}
	}

	return true;
}

int32 freeModel(
	const char *name,
	Model *models,
	uint32 *numModels)
{
	for (uint32 i = 0; i < *numModels; i++)
	{
		Model *model = &models[i];

		if (strcmp(model->name, name) == 0)
		{
			if (--(model->refCount) == 0)
			{
				free(model->name);

				glBindVertexArray(model->mesh.vertexArray);

				glDeleteBuffers(1, &model->mesh.colorBuffer);
				glDeleteBuffers(1, &model->mesh.positionBuffer);
				glDeleteBuffers(1, &model->mesh.normalBuffer);
				glDeleteBuffers(1, &model->mesh.tangentBuffer);
				glDeleteBuffers(1, &model->mesh.bitangentBuffer);
				glDeleteBuffers(1, &model->mesh.uvBuffer);
				glDeleteBuffers(1, &model->mesh.indexBuffer);

				glBindVertexArray(0);
				glDeleteVertexArrays(1, &model->mesh.vertexArray);

				for (uint32 j = 0; j < model->numMaterials; j++)
				{
					free(model->materials[i].diffuseTexture);
					free(model->materials[i].specularTexture);
					free(model->materials[i].normalMap);
					free(model->materials[i].emissiveMap);
				}

				free(model->materials);

				Model *resizedModels = malloc(--(*numModels) * sizeof(Model));
				memcpy(resizedModels, models, i * sizeof(Model));

				if (i < *numModels)
				{
					memcpy(
						resizedModels + i * sizeof(Model),
						model + sizeof(Model),
						(*numModels - i) * sizeof(Model)
					);
				}

				free(models);
				models = resizedModels;
			}

			break;
		}
	}

	return 0;
}

int32 freeTexture(
	const char *name,
	Texture *textures,
	uint32 *numTextures) 
{
	for (uint32 i = 0; i < *numTextures; i++)
	{
		Texture *texture = &textures[i];

		if (strcmp(texture->name, name) == 0)
		{
			if (--(texture->refCount) == 0)
			{
				free(texture->name);

				glDeleteTextures(1, &texture->id);

				Texture *resizedTextures = malloc(--(*numTextures) * sizeof(Texture));
				memcpy(resizedTextures, textures, i * sizeof(Texture));

				if (i < *numTextures)
				{
					memcpy(
						resizedTextures + i * sizeof(Texture),
						texture + sizeof(Texture),
						(*numTextures - i) * sizeof(Texture)
					);
				}

				free(textures);
				textures = resizedTextures;
			}

			break;
		}
	}

	return 0;
}

int32 unloadScene(Scene **scene)
{
	return 0;
}
