#include "asset_management/asset_manager_types.h"
#include "asset_management/mesh.h"

#include "core/log.h"

#include <malloc.h>
#include <stddef.h>

int32 loadMesh(Mesh *mesh, FILE *file)
{
	LOG("Loading mesh...\n");
	LOG("Loading mesh data...\n");

	uint32 numVertices;
	fread(&numVertices, sizeof(uint32), 1, file);

	Vertex *vertices = calloc(numVertices, sizeof(Vertex));
	fread(vertices, numVertices, sizeof(Vertex), file);

	uint32 numIndices;
	fread(&numIndices, sizeof(uint32), 1, file);

	uint32 *indices = calloc(numIndices, sizeof(uint32));
	fread(indices, numIndices, sizeof(uint32), file);

	LOG("Successfully loaded mesh data\n");
	LOG("Transferring mesh data onto GPU...\n");

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
	glVertexAttribPointer(
		bufferIndex++,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(GLvoid*)offsetof(Vertex, materialUV));
	glVertexAttribPointer(
		bufferIndex++,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(GLvoid*)offsetof(Vertex, maskUV));
	glVertexAttribIPointer(
		bufferIndex++,
		NUM_BONES,
		GL_UNSIGNED_INT,
		sizeof(Vertex),
		(GLvoid*)offsetof(Vertex, bones));
	glVertexAttribPointer(
		bufferIndex++,
		NUM_BONES,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(GLvoid*)offsetof(Vertex, weights));

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

	LOG("Successfully transferred mesh data onto GPU\n");
	LOG("Vertex Count: %d\n", numVertices);
	LOG("Triangle Count: %d\n", numIndices / 3);
	LOG("Successfully loaded mesh\n");

	return 0;
}

void freeMesh(Mesh *mesh)
{
	glBindVertexArray(mesh->vertexArray);
	glDeleteBuffers(1, &mesh->vertexBuffer);
	glDeleteBuffers(1, &mesh->indexBuffer);
	glBindVertexArray(0);

	glDeleteVertexArrays(1, &mesh->vertexArray);
}