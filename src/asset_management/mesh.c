#include "asset_management/mesh.h"

#include <assimp/scene.h>

int32 loadMesh(const struct aiMesh *mesh, MeshData *meshData)
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

int32 freeMesh(Mesh *mesh)
{
	glBindVertexArray(mesh->vertexArray);

	glDeleteBuffers(1, &mesh->colorBuffer);
	glDeleteBuffers(1, &mesh->positionBuffer);
	glDeleteBuffers(1, &mesh->normalBuffer);
	glDeleteBuffers(1, &mesh->tangentBuffer);
	glDeleteBuffers(1, &mesh->bitangentBuffer);
	glDeleteBuffers(1, &mesh->uvBuffer);
	glDeleteBuffers(1, &mesh->indexBuffer);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &mesh->vertexArray);
	
	return 0;
}
