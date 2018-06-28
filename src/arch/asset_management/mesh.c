#include "asset_management/mesh.h"
#include "asset_management/asset_manager_types.h"

#include "model-utility/mesh_exporter.h"

#include "file/utilities.h"

#include <stdio.h>
#include <malloc.h>
#include <stddef.h>
#include <string.h>

int32 loadMesh(Model *model)
{
	char *modelFolder = getFullFilePath(model->name, NULL, "resources/models");
	char *modelFilename = getFullFilePath(model->name, NULL, modelFolder);

	if (exportMesh(modelFilename) == -1)
	{
		free(modelFilename);
		free(modelFolder);
		return -1;
	}

	free(modelFilename);

	char *meshFilename = getFullFilePath(model->name, "mesh", modelFolder);
	free(modelFolder);

	FILE *file = fopen(meshFilename, "rb");
	free(meshFilename);

	if (file)
	{
		uint32 numSubsets;
		fread(&numSubsets, sizeof(uint32), 1, file);

		model->meshes = calloc(numSubsets, sizeof(Mesh));

		for (uint32 i = 0; i < numSubsets; i++)
		{
			char *subsetName = readString(file);
			printf("Loading subset mesh (%s)...\n", subsetName);
			printf("Loading subset mesh data...\n");

			uint32 numVertices;
			fread(&numVertices, sizeof(uint32), 1, file);

			Vertex *vertices = calloc(numVertices, sizeof(Vertex));
			fread(vertices, numVertices, sizeof(Vertex), file);

			uint32 numIndices;
			fread(&numIndices, sizeof(uint32), 1, file);

			uint32 *indices = calloc(numIndices, sizeof(uint32));
			fread(indices, numIndices, sizeof(uint32), file);

			printf("Successfully loaded subset mesh data\n");
			printf("Loading subset mesh onto GPU...\n");

			Mesh *mesh = &model->meshes[i];

			for (uint32 j = 0; j < model->numSubsets; j++)
			{
				if (!strcmp(model->materials[j].name, subsetName))
				{
					mesh->materialIndex = j;
					break;
				}
			}

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

			for (uint32 j = 0; j < MATERIAL_COMPONENT_TYPE_COUNT; j++)
			{
				glVertexAttribPointer(
					bufferIndex++,
					2,
					GL_FLOAT,
					GL_FALSE,
					sizeof(Vertex),
					(GLvoid*)offsetof(Vertex, uv[j]));
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
			printf("Successfully loaded subset mesh (%s)\n", subsetName);
			free(subsetName);
		}

		fclose(file);
	}
	else
	{
		printf("Failed to open %s\n", meshFilename);
		return -1;
	}

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
