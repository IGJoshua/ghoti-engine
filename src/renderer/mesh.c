#include "renderer/mesh.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <kazmath/vec4.h>
#include <kazmath/vec3.h>
#include <kazmath/vec2.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <malloc.h>
#include <string.h>

int32 loadMesh(Mesh **m, const char *filename, const char *meshName)
{
	*m = malloc(sizeof(Mesh));
#ifdef _DEBUG
	memset(*m, 0, sizeof(Mesh));
#endif

	const struct aiScene *scene = aiImportFile(filename, aiProcessPreset_TargetRealtime_Quality & ~aiProcess_SplitLargeMeshes & ~aiProcess_Triangulate);

	uint32 meshCount = scene->mNumMeshes;
	uint32 rootMeshCount = scene->mRootNode->mNumMeshes;
	//uint32 rootMeshIndex = scene->mRootNode->mMeshes[0];
	const struct aiMesh * mesh = scene->mMeshes[1];

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

		uvs[i].y = 0.0f;//mesh->mTextureCoords[i]->x;
		uvs[i].y = 0.0f;//mesh->mTextureCoords[i]->y;
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
