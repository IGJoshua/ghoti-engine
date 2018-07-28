#include "defines.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/texture.h"

#include "renderer/renderer_types.h"
#include "renderer/shader.h"

#include "components/component_types.h"
#include "components/rigid_body.h"
#include "components/transform.h"
#include "components/camera.h"

#include <ode/ode.h>

#include <kazmath/mat3.h>
#include <kazmath/mat4.h>

#define SCENE_BUCKET_COUNT 7

#define INDEX(x, y, max) ((y) * (max) + (x))

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

internal UUID heightmapComponentID = {};
internal UUID cameraComponentID = {};
internal UUID transformComponentID = {};

internal bool rendererActive = false;

internal HashMap heightmapModels = 0;

internal Shader vertShader = {};
internal Shader fragShader = {};

internal ShaderPipeline pipeline = {};

internal Uniform modelUniform = {};
internal Uniform viewUniform = {};
internal Uniform projectionUniform = {};
internal Uniform textureUniforms[1] = {};

internal
void initRenderHeightmapSystem(Scene *scene)
{
	if (!rendererActive)
	{
		// TODO: create and compile the shader pipeline
		if (compileShaderFromFile(
				"resources/shaders/base.vert",
				SHADER_VERTEX,
				&vertShader) == -1)
		{
			LOG("Unable to compile vertex shader for heightmap rendering\n");
		}

		if (compileShaderFromFile(
				"resources/shaders/color.frag",
				SHADER_FRAGMENT,
				&fragShader) == -1)
		{
			LOG("Unable to compile fragment shader for heightmap rendering\n");
		}

		Shader *program[2];
		program[0] = &vertShader;
		program[1] = &fragShader;

		if (composeShaderPipeline(program, 2, &pipeline) == -1)
		{
			LOG("Unable to compose shader pipeline for heightmap rendering\n");
		}

		freeShader(vertShader);
		freeShader(fragShader);
		free(pipeline.shaders);
		pipeline.shaderCount = 0;

		if (getUniform(pipeline, "model", UNIFORM_MAT4, &modelUniform) == -1)
		{
			LOG("Unalbe to get model uniform for heightmap rendering\n");
		}

		if (getUniform(pipeline, "view", UNIFORM_MAT4, &viewUniform) == -1)
		{
			LOG("Unalbe to get view uniform for heightmap rendering\n");
		}

		if (getUniform(
				pipeline,
				"projection",
				UNIFORM_MAT4,
				&projectionUniform) == -1)
		{
			LOG("Unalbe to get projection uniform for heightmap rendering\n");
		}
	}

	HashMap map = createHashMap(
		sizeof(UUID),
		sizeof(Mesh),
		SCENE_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	hashMapInsert(heightmapModels, &scene, &map);

	// iterate over all the heightmaps and load the images and create geometry for it
	for (ComponentDataTableIterator itr = cdtGetIterator(
			 *(ComponentDataTable **)hashMapGetKey(
				 scene->componentTypes,
				 &heightmapComponentID));
		 !cdtIteratorAtEnd(itr);
		 cdtMoveIterator(&itr))
	{
		HeightmapComponent *heightmap = cdtIteratorGetData(itr);

		ILuint imageID;
		loadTextureWithFormat(
			heightmap->textureName,
			TEXTURE_FORMAT_R8,
			false,
			&imageID);
		ilBindImage(imageID);
		uint32 imageWidth = ilGetInteger(IL_IMAGE_WIDTH);
		uint32 imageHeight = ilGetInteger(IL_IMAGE_HEIGHT);
		uint8 *imageData = ilGetData();

		// TODO: create grid for size of heightmap
		uint32 numVerts = (heightmap->sizeX + 1) * (heightmap->sizeZ + 1);
		Vertex *verts = calloc(sizeof(Vertex), numVerts);

		// TODO: Create verts at the correct heights
		for (uint32 x = 0; x <= heightmap->sizeX; ++x)
		{
			for (uint32 z = 0; z <= heightmap->sizeZ; ++z)
			{
				uint32 vertIndex = INDEX(x, z, heightmap->sizeX + 1);

				Vertex vert = {};

				kmVec2 uv;
				kmVec2Fill(
					&uv,
					(real32)x / (real32)heightmap->sizeX + 1,
					(real32)z / (real32)heightmap->sizeZ + 1);
				uint32 imageIndex = INDEX(
					(uint32)(uv.x * imageWidth),
					(uint32)(uv.y * imageHeight),
					imageWidth);

				vert.position.y =
					imageData[imageIndex] / 255.0f
					* heightmap->maxHeight;
				vert.position.x =
					(x - 0.5f * (heightmap->sizeX + 1))
					* heightmap->unitsPerTile;
				vert.position.z =
					(z - 0.5f * (heightmap->sizeZ + 1))
					* heightmap->unitsPerTile;

				vert.uv[0].x = uv.x * heightmap->uvScaleX;
				vert.uv[0].y = uv.y * heightmap->uvScaleZ;

				verts[vertIndex] = vert;
			}
		}

		// TODO: Create valid normals for all the verts
		for (uint32 x = 0; x <= heightmap->sizeX; ++x)
		{
			for (uint32 z = 0; z <= heightmap->sizeZ; ++z)
			{
				// Get all the stuff around this point and get a normal from it
				uint32 tempx = MAX(1, MIN(heightmap->sizeX, x));
				uint32 tempz = MAX(1, MIN(heightmap->sizeZ, z));

				real32 hl =
					verts[
						INDEX(tempx - 1, tempz, heightmap->sizeX + 1)
					].position.y;
				real32 hr =
					verts[
						INDEX(tempx + 1, tempz, heightmap->sizeX + 1)
					].position.y;
				real32 hd =
					verts[
						INDEX(tempx, tempz + 1, heightmap->sizeX + 1)
					].position.y;
				real32 hu =
					verts[
						INDEX(tempx, tempz - 1, heightmap->sizeX + 1)
					].position.y;

				kmVec3 normal;
				kmVec3Fill(&normal, hl - hr, 2, hd - hu);
				kmVec3Normalize(&normal, &normal);

				uint32 vertIndex = INDEX(x, z, heightmap->sizeX + 1);
				Vertex *vert = &verts[vertIndex];

				// TODO: Calculate tangent and bitangent for normal mapping
				vert->normal = normal;
			}
		}

		ilDeleteImage(imageID);

		// Create the index buffer
		uint32 numIndices =
			(((heightmap->sizeX + 1) * 2) + 2)
			* heightmap->sizeZ - 1;
		uint32 *indices = calloc(
			sizeof(uint32),
			numIndices);

		uint32 vertCol = 0;
		uint32 row = 0;
		uint32 index = 0;

		while (row < heightmap->sizeZ)
		{
			// Do a triangle strip
			if (vertCol <= heightmap->sizeX)
			{
				indices[index++] = INDEX(
					vertCol,
					row,
					heightmap->sizeX + 1);

				indices[index++] = INDEX(
					++vertCol,
					row + 1,
					heightmap->sizeX + 1);
			}
			// If you're past the end of the row, do a degenerate triangle
			else
			{
				indices[index] = indices[index - 1];
				++index;

				vertCol = 0;
				++row;

				indices[index++] = INDEX(
					vertCol,
					row,
					heightmap->sizeX + 1);
			}
		}

		// Upload the model to the GPU
		Mesh m = {};

		m.materialIndex = -1;

		glGenBuffers(1, &m.vertexBuffer);
		glGenVertexArrays(1, &m.vertexArray);

		uint32 bufferIndex = 0;

		glBindBuffer(GL_ARRAY_BUFFER, m.vertexBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			sizeof(Vertex) * numVerts,
			verts,
			GL_STATIC_DRAW);

		glBindVertexArray(m.vertexArray);
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

		glGenBuffers(1, &m.indexBuffer);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.indexBuffer);
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			sizeof(uint32) * numIndices,
			indices,
			GL_STATIC_DRAW);

		m.numIndices = numIndices;

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		free(verts);
		free(indices);

		hashMapInsert(map, &scene, &m);
	}
}

extern real64 alpha;

internal
void beginRenderHeightmapSystem(Scene *scene, real64 dt)
{
	cameraSetUniforms(scene, viewUniform, projectionUniform, pipeline);

	for (GLint i = 0; i < MATERIAL_COMPONENT_TYPE_COUNT; i++)
	{
		if (textureUniforms[i].type != UNIFORM_INVALID)
		{
			if (setUniform(textureUniforms[i], &i) == -1)
			{
				LOG("Unable to set texture uniform %d\n", i);
			}
		}
	}
}

internal
void runRenderHeightmapSystem(Scene *scene, UUID entityID, real64 dt)
{

}

internal
void shutdownRenderHeightmapSystem(Scene *scene)
{
	// TODO: free any information in the hash map with the scene, and delete the hash map entry
	HashMap *map = hashMapGetKey(heightmapModels, &scene);

	for (HashMapIterator itr = hashMapGetIterator(*map);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		Mesh *m = hashMapIteratorGetValue(itr);

		glBindVertexArray(m->vertexArray);
		glDeleteBuffers(1, &m->vertexBuffer);
		glDeleteBuffers(1, &m->indexBuffer);
		glBindVertexArray(0);

		glDeleteVertexArrays(1, &m->vertexArray);
	}

	hashMapClear(*map);
	freeHashMap(map);
	hashMapDeleteKey(heightmapModels, &scene);
}

internal
int32 ptrEq(void *thing1, void *thing2)
{
	return *(uint64*)thing1 != *(uint64*)thing2;
}

System createRenderHeightmapSystem(void)
{
	heightmapComponentID = idFromName("heightmap");
	cameraComponentID = idFromName("camera");
	transformComponentID = idFromName("transform");

	heightmapModels = createHashMap(
		sizeof(Scene *),
		sizeof(HashMap),
		SCENE_BUCKET_COUNT,
		&ptrEq);

	System ret = {};

	ret.componentTypes = createList(sizeof(UUID));
	listPushFront(&ret.componentTypes, &transformComponentID);
	listPushFront(&ret.componentTypes, &heightmapComponentID);

	ret.init = &initRenderHeightmapSystem;
	ret.begin = &beginRenderHeightmapSystem;
	ret.run = &runRenderHeightmapSystem;
	ret.shutdown = &shutdownRenderHeightmapSystem;

	return ret;
}
