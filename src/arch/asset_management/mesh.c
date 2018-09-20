#include "asset_management/asset_manager_types.h"
#include "asset_management/mesh.h"

#include "core/log.h"

#include <malloc.h>
#include <stddef.h>

void loadMesh(Mesh *mesh, FILE *file)
{
	LOG("Loading mesh data...\n");

	fread(&mesh->numVertices, sizeof(uint32), 1, file);

	mesh->vertices = calloc(mesh->numVertices, sizeof(Vertex));
	fread(mesh->vertices, mesh->numVertices, sizeof(Vertex), file);

	fread(&mesh->numIndices, sizeof(uint32), 1, file);

	mesh->indices = calloc(mesh->numIndices, sizeof(uint32));
	fread(mesh->indices, mesh->numIndices, sizeof(uint32), file);

	LOG("Successfully loaded mesh data\n");

	uploadMeshToGPU(mesh);
}

void uploadMeshToGPU(Mesh *mesh)
{
	LOG("Transferring mesh data onto GPU...\n");

	glGenBuffers(1, &mesh->vertexBuffer);
	glGenVertexArrays(1, &mesh->vertexArray);

	uint32 bufferIndex = 0;

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(Vertex) * mesh->numVertices,
		mesh->vertices,
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
		sizeof(uint32) * mesh->numIndices,
		mesh->indices,
		GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	free(mesh->vertices);
	free(mesh->indices);

	LOG("Successfully transferred mesh data onto GPU\n");
	LOG("Vertex Count: %d\n", mesh->numVertices);
	LOG("Triangle Count: %d\n", mesh->numIndices / 3);
}

void freeMesh(Mesh *mesh)
{
	glBindVertexArray(mesh->vertexArray);
	glDeleteBuffers(1, &mesh->vertexBuffer);
	glDeleteBuffers(1, &mesh->indexBuffer);
	glBindVertexArray(0);

	glDeleteVertexArrays(1, &mesh->vertexArray);
}