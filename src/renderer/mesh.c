#include "renderer/mesh.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <kazmath/vec4.h>
#include <kazmath/vec3.h>
#include <kazmath/vec2.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/material.h>

#include <IL/il.h>
#include <IL/ilu.h>

#include <malloc.h>
#include <string.h>

int32 loadMesh(const struct aiScene *scene, Mesh **m, const char *meshName, const struct aiMesh **out)
{
	*m = malloc(sizeof(Mesh));
	#ifdef _DEBUG
		memset(*m, 0, sizeof(Mesh));
	#endif

	uint32 meshCount = scene->mNumMeshes;

	const struct aiMesh * mesh;
	if (!meshName)
	{
		mesh = scene->mMeshes[0];
	}
	else
	{
		for (uint32 i = 0; i < meshCount; ++i)
		{
			if (scene->mMeshes[i]->mName.data == meshName)
			{
				mesh = scene->mMeshes[i];
			}
		}
	}

	if (out)
	{
		*out = mesh;
	}

	const struct aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

	struct aiString textureName;
	if (aiGetMaterialString(
		material,
		AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0),
		&textureName) == AI_SUCCESS)
	{
		printf("Texture Name: %s\n", textureName.data);
	}
	else
	{
		printf("Error Getting Texture Name\n");
	}

	ILuint devilID;
	ilGenImages(1, &devilID);
	ilBindImage(devilID);

	char texturePath[128] = "resources/textures/";
	strcat(texturePath, textureName.data);
	printf("Texture Path: %s\n", texturePath);
	ilLoadImage(texturePath);

	printf("IL Load Image: %s\n", iluErrorString(ilGetError()));

	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	printf("IL Convert Image: %s\n", iluErrorString(ilGetError()));

	GLuint textureID;
	glGenTextures(1, &textureID);

	printf("Value of the location of the texture: %d\n", textureID);

	glActiveTexture(GL_TEXTURE0);
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

	printf("Create GL Texture2D: %s\n", gluErrorString(glGetError()));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);

	printf("Generate Mipmaps: %s\n", gluErrorString(glGetError()));

	// TODO: Change this to use the stride and offset
	// to allow the entire thing to be stored in one array
	uint32 numVertices = mesh->mNumVertices;
	kmVec4 *colors = malloc(numVertices * sizeof(kmVec4));
	kmVec3 *positions = malloc(numVertices * sizeof(kmVec3));
	kmVec3 *normals = malloc(numVertices * sizeof(kmVec3));
	kmVec2 *uvs = malloc(numVertices * sizeof(kmVec2));

	for (uint32 i = 0; i < mesh->mNumVertices; ++i)
	{
		colors[i].x = 0.0f;
		colors[i].y = 0.0f;
		colors[i].z = 0.0f;
		colors[i].w = 1.0f;

		positions[i].x = mesh->mVertices[i].x;
		positions[i].y = mesh->mVertices[i].y;
		positions[i].z = mesh->mVertices[i].z;

		normals[i].x = mesh->mNormals[i].x;
		normals[i].y = mesh->mNormals[i].y;
		normals[i].z = mesh->mNormals[i].z;

		uvs[i].x = mesh->mTextureCoords[0][i].x;
		uvs[i].y = mesh->mTextureCoords[0][i].y;
	}

	uint32 numIndices = mesh->mNumFaces * 3;
	uint32 *indices = malloc(numIndices * sizeof(uint32));

	(*m)->numIndices = numIndices;

	for (uint32 i = 0; i < mesh->mNumFaces; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			indices[i * 3 + j] = mesh->mFaces[i].mIndices[j];
		}
	}

	aiReleaseImport(scene);

	// Upload to the gpu
	GLuint vao;
	glGenVertexArrays(1, &vao);

	(*m)->vertexArray = vao;

	glBindVertexArray(vao);

	GLuint colorBuffer;
	glGenBuffers(1, &colorBuffer);

	(*m)->colorBuffer = colorBuffer;

	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(kmVec4) * numVertices, colors, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint positionBuffer;
	glGenBuffers(1, &positionBuffer);

	(*m)->positionBuffer = positionBuffer;

	glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(kmVec4) * numVertices, positions, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint normalBuffer;
	glGenBuffers(1, &normalBuffer);

	(*m)->normalBuffer = normalBuffer;

	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(kmVec4) * numVertices, normals, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, 0, 0);

	GLuint uvBuffer;
	glGenBuffers(1, &uvBuffer);

	(*m)->uvBuffer = uvBuffer;

	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(kmVec4) * numVertices, uvs, GL_STATIC_DRAW);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint indexBuffer;
	glGenBuffers(1, &indexBuffer);

	(*m)->indexBuffer = indexBuffer;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * numIndices, indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	free(colors);
	colors = 0;
	free(positions);
	positions = 0;
	free(normals);
	normals = 0;
	free(uvs);
	uvs = 0;
	free(indices);
	indices = 0;

	ilDeleteImages(1, &devilID);

	return 0;
}

void freeMesh(Mesh **m)
{
	glBindVertexArray((*m)->vertexArray);

	glDeleteBuffers(1, &(*m)->colorBuffer);
	glDeleteBuffers(1, &(*m)->positionBuffer);
	glDeleteBuffers(1, &(*m)->normalBuffer);
	glDeleteBuffers(1, &(*m)->uvBuffer);
	glDeleteBuffers(1, &(*m)->indexBuffer);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &(*m)->vertexArray);

	//glDeleteTextures(1, &textureID);

	free(*m);
	*m = 0;
}

void renderMesh(Mesh *m)
{
	glBindVertexArray(m->vertexArray);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->indexBuffer);
	glDrawElements(GL_TRIANGLES, m->numIndices, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glBindVertexArray(0);
}
