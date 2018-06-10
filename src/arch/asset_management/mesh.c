#include "asset_management/mesh.h"

#include <malloc.h>
#include <stddef.h>

int32 loadMesh(
	const struct aiMesh *meshData,
	const Material *material,
	Mesh *mesh)
{
	uint32 numVertices = 0;
	Vertex *vertices = calloc(meshData->mNumVertices, sizeof(Vertex));
	uint32 numIndices = 0;
	uint32 *indices = calloc(meshData->mNumFaces * 3, sizeof(uint32));

	uint32 i, j;

	printf("Loading subset mesh data...\n");

	for (i = 0; i < meshData->mNumVertices; i++, numVertices++)
	{
		Vertex *vertex = &vertices[numVertices];

		const struct aiVector3D *positions = meshData->mVertices;

		vertex->position.x = positions[i].x;
		vertex->position.y = positions[i].y;
		vertex->position.z = positions[i].z;

		const struct aiColor4D *colors = meshData->mColors[0];

		if (colors)
		{
			vertex->color.x = colors[i].r;
			vertex->color.y = colors[i].g;
			vertex->color.z = colors[i].b;
			vertex->color.w = colors[i].a;
		}
		else
		{
			vertex->color.x = 0.0f;
			vertex->color.y = 0.0f;
			vertex->color.z = 0.0f;
			vertex->color.w = 1.0f;
		}

		const struct aiVector3D *normals = meshData->mNormals;

		vertex->normal.x = normals[i].x;
		vertex->normal.y = normals[i].y;
		vertex->normal.z = normals[i].z;

		const struct aiVector3D *tangents = meshData->mTangents;

		vertex->tangent.x = tangents[i].x;
		vertex->tangent.y = tangents[i].y;
		vertex->tangent.z = tangents[i].z;

		const struct aiVector3D *bitangents = meshData->mBitangents;

		vertex->bitangent.x = bitangents[i].x;
		vertex->bitangent.y = bitangents[i].y;
		vertex->bitangent.z = bitangents[i].z;

		for (j = 0; j < MATERIAL_COMPONENT_TYPE_COUNT; j++)
		{
			const struct aiVector3D *uvs =
				meshData->mTextureCoords[material->components[j].uvMap];

			switch ((MaterialComponentType)j)
			{
				case MATERIAL_COMPONENT_TYPE_DIFFUSE:
				case MATERIAL_COMPONENT_TYPE_SPECULAR:
				case MATERIAL_COMPONENT_TYPE_NORMAL:
				case MATERIAL_COMPONENT_TYPE_EMISSIVE:
					vertex->uv[j].x = uvs[i].x;
					vertex->uv[j].y = 1.0f - uvs[i].y;
					break;
				default:
					break;
			}
		}
	}

	for (i = 0; i < meshData->mNumFaces; ++i)
	{
		for (j = 0; j < 3; ++j)
		{
			indices[numIndices++] =
				meshData->mFaces[i].mIndices[j];
		}
	}

	printf("Successfully loaded subset mesh data\n");
	printf("Loading subset mesh onto GPU...\n");

	glGenBuffers(1, &mesh->vertexBuffer);
	glGenVertexArrays(1, &mesh->vertexArray);

	uint32 bufferIndex = 0;

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(Vertex) * numVertices,
		vertices,
		GL_STATIC_DRAW);

	glBindVertexArray(mesh->vertexArray);
	glVertexAttribPointer(
		bufferIndex++,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(GLvoid*)offsetof(Vertex, position));
	glVertexAttribPointer(
		bufferIndex++,
		4,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(GLvoid*)offsetof(Vertex, color));
	glVertexAttribPointer(
		bufferIndex++,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(GLvoid*)offsetof(Vertex, normal));
	glVertexAttribPointer(
		bufferIndex++,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(GLvoid*)offsetof(Vertex, tangent));
	glVertexAttribPointer(
		bufferIndex++,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(GLvoid*)offsetof(Vertex, bitangent));

	for (i = 0; i < MATERIAL_COMPONENT_TYPE_COUNT; i++)
	{
		glVertexAttribPointer(
			bufferIndex++,
			2,
			GL_FLOAT,
			GL_FALSE,
			sizeof(Vertex),
			(GLvoid*)offsetof(Vertex, uv[i]));
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenBuffers(1, &mesh->indexBuffer);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(uint32) * numIndices,
		indices,
		GL_STATIC_DRAW);

	mesh->numIndices = numIndices;

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	free(vertices);
	free(indices);

	printf("Successfully loaded subset mesh onto GPU\n");
	printf("Vertex Count: %d\n", numVertices);
	printf("Index Count: %d\n", numIndices);

	return 0;
}

int32 freeMesh(Mesh *mesh)
{
	glBindVertexArray(mesh->vertexArray);
	glDeleteBuffers(1, &mesh->vertexBuffer);
	glDeleteBuffers(1, &mesh->indexBuffer);
	glBindVertexArray(0);

	glDeleteVertexArrays(1, &mesh->vertexArray);

	return 0;
}
