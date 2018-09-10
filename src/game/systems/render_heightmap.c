#include "defines.h"

#include "core/log.h"

#include "file/utilities.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/material.h"
#include "asset_management/texture.h"

#include "renderer/renderer_types.h"
#include "renderer/renderer_utilities.h"
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

internal uint32 rendererRefCount = 0;

internal HashMap heightmapModels = NULL;

internal GLuint shaderProgram = 0;

internal Uniform modelUniform = {};
internal Uniform viewUniform = {};
internal Uniform projectionUniform = {};

internal Uniform materialUniform = {};
internal Uniform materialValuesUniform = {};

typedef struct heightmap_model_t
{
	Mesh mesh;
	Material material;
} HeightmapModel;

internal int32 createHeightmapMaterial(UUID name, Material *material);

internal
int32 ptrEq(void *thing1, void *thing2)
{
	return *(uint64*)thing1 != *(uint64*)thing2;
}

internal
void initRenderHeightmapSystem(Scene *scene)
{
	if (rendererRefCount == 0)
	{
		LOG("Initializing heightmap renderer...\n");

		heightmapModels = createHashMap(
			sizeof(Scene *),
			sizeof(HashMap),
			SCENE_BUCKET_COUNT,
			&ptrEq);

		createShaderProgram(
			"resources/shaders/model.vert",
			NULL,
			NULL,
			NULL,
			"resources/shaders/model.frag",
			NULL,
			&shaderProgram);

		getUniform(
			shaderProgram,
			"model",
			UNIFORM_MAT4,
			&modelUniform);
		getUniform(shaderProgram, "view", UNIFORM_MAT4, &viewUniform);
		getUniform(
			shaderProgram,
			"projection",
			UNIFORM_MAT4,
			&projectionUniform);
		getUniform(
			shaderProgram,
			"material",
			UNIFORM_TEXTURE_2D,
			&materialUniform);
		getUniform(
			shaderProgram,
			"materialValues",
			UNIFORM_TEXTURE_2D,
			&materialValuesUniform);

		LOG("Successfully initialized heightmap renderer\n");
	}

	rendererRefCount++;

	HashMap map = createHashMap(
		sizeof(UUID),
		sizeof(HeightmapModel),
		SCENE_BUCKET_COUNT,
		(ComparisonOp)&strcmp);
	hashMapInsert(heightmapModels, &scene, &map);

	// iterate over all the heightmaps and load the images and create geometry for it
	for (ComponentDataTableIterator itr = cdtGetIterator(
			 *(ComponentDataTable **)hashMapGetData(
				 scene->componentTypes,
				 &heightmapComponentID));
		 !cdtIteratorAtEnd(itr);
		 cdtMoveIterator(&itr))
	{
		UUID entityID = cdtIteratorGetUUID(itr);
		HeightmapComponent *heightmap = cdtIteratorGetData(itr);
		TransformComponent *transform = sceneGetComponentFromEntity(
			scene,
			entityID,
			transformComponentID);

		char *heightmapFilename = getFullFilePath(
			heightmap->heightmapName,
			NULL,
			"resources/heightmaps");
		char *fullHeightmapFilename = getFullTextureFilename(heightmapFilename);
		free(heightmapFilename);

		ILuint imageID;
		if (loadTextureData(
				fullHeightmapFilename,
				TEXTURE_FORMAT_R8,
				&imageID) == -1)
		{
			LOG("Unable to load texture %s, heightmap is broken\n",
				heightmap->heightmapName);
			free(fullHeightmapFilename);
			continue;
		}

		free(fullHeightmapFilename);

		ilBindImage(imageID);
		uint32 imageWidth = ilGetInteger(IL_IMAGE_WIDTH);
		uint32 imageHeight = ilGetInteger(IL_IMAGE_HEIGHT);
		uint8 *imageData = ilGetData();

		// create grid for size of heightmap
		uint32 numVerts = (heightmap->sizeX + 1) * (heightmap->sizeZ + 1);
		Vertex *verts = calloc(numVerts, sizeof(Vertex));

		// Load the image into ODE
		dHeightfieldDataID heightfieldData = dGeomHeightfieldDataCreate();
		dGeomHeightfieldDataBuildByte(
			heightfieldData,
			imageData,
			1,
			heightmap->sizeX * heightmap->unitsPerTile,
			heightmap->sizeZ * heightmap->unitsPerTile,
			imageWidth,
			imageHeight,
			heightmap->maxHeight / 255.0f,
			0,
			5,
			0);
		dGeomHeightfieldDataSetBounds(
			heightfieldData,
			0,
			255);
		heightmap->heightfieldGeom = dCreateHeightfield(
			scene->physicsSpace,
			heightfieldData,
			1);
		dGeomHeightfieldSetHeightfieldData(
			heightmap->heightfieldGeom,
			heightfieldData);

		// Position the heightfield correctly
		dGeomSetPosition(
			heightmap->heightfieldGeom,
			transform->position.x,
			transform->position.y,
			transform->position.z);

		dQuaternion q;
		q[0] = transform->rotation.w;
		q[1] = transform->rotation.x;
		q[2] = transform->rotation.y;
		q[3] = transform->rotation.z;

		dGeomSetQuaternion(heightmap->heightfieldGeom, q);

		void *userData = calloc(1, sizeof(UUID));
		memcpy(userData, &entityID, sizeof(UUID));
		dGeomSetData(heightmap->heightfieldGeom, userData);

		// Create verts at the correct heights (currently it's doing it wrong)
		for (uint32 x = 0; x <= heightmap->sizeX; ++x)
		{
			for (uint32 z = 0; z <= heightmap->sizeZ; ++z)
			{
				uint32 vertIndex = INDEX(x, z, heightmap->sizeX + 1);

				Vertex vert = {};

				kmVec2 uv;
				kmVec2Fill(
					&uv,
					(real32)x / ((real32)heightmap->sizeX + 1),
					(real32)z / ((real32)heightmap->sizeZ + 1));
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

				vert.materialUV.x = uv.x * heightmap->uvScaleX;
				vert.materialUV.y = uv.y * heightmap->uvScaleZ;

				verts[vertIndex] = vert;
			}
		}

		// TODO: Fix generation of normals
		// // Create valid normals for all the verts
		// for (uint32 x = 0; x <= heightmap->sizeX; ++x)
		// {
		// 	for (uint32 z = 0; z <= heightmap->sizeZ; ++z)
		// 	{
		// 		// Get all the stuff around this point and get a normal from it
		// 		uint32 tempx = MAX(1, MIN(heightmap->sizeX, x));
		// 		uint32 tempz = MAX(1, MIN(heightmap->sizeZ, z));

		// 		real32 hl =
		// 			verts[
		// 				INDEX(tempx - 1, tempz, heightmap->sizeX + 1)
		// 			].position.y;
		// 		real32 hr =
		// 			verts[
		// 				INDEX(tempx + 1, tempz, heightmap->sizeX + 1)
		// 			].position.y;
		// 		real32 hd =
		// 			verts[
		// 				INDEX(tempx, tempz + 1, heightmap->sizeX + 1)
		// 			].position.y;
		// 		real32 hu =
		// 			verts[
		// 				INDEX(tempx, tempz - 1, heightmap->sizeX + 1)
		// 			].position.y;

		// 		kmVec3 normal;
		// 		kmVec3Fill(&normal, hl - hr, 2, hd - hu);
		// 		kmVec3Normalize(&normal, &normal);

		// 		uint32 vertIndex = INDEX(x, z, heightmap->sizeX + 1);
		// 		Vertex *vert = &verts[vertIndex];

		// 		// TODO: Calculate tangent and bitangent for normal mapping
		// 		vert->normal = normal;
		// 	}
		// }

		for (uint32 i = 0; i < numVerts; i++)
		{
			Vertex *vert = &verts[i];
			kmVec3Fill(&vert->normal, 0.0f, 1.0f, 0.0f);
		}

		ilDeleteImage(imageID);

		// Create the index buffer
		uint32 numIndices = (heightmap->sizeX * heightmap->sizeZ) * 6;
		uint32 *indices = calloc(sizeof(uint32), numIndices);

		uint32 index = 0;

		for (uint32 triX = 0; triX < heightmap->sizeX; ++triX)
		{
			for (uint32 triZ = 0; triZ < heightmap->sizeZ; ++triZ)
			{
				// Create indices for the first triangle
				// (-x, -z), (-x, z), (x, z)
				indices[index++] = INDEX(triX, triZ, heightmap->sizeX + 1);
				indices[index++] = INDEX(triX, triZ + 1, heightmap->sizeX + 1);
				indices[index++] = INDEX(
					triX + 1, triZ + 1, heightmap->sizeX + 1);

				// Create indices for the second triangle
				// (x, z), (x, -z), (-x, -z)
				indices[index++] = INDEX(
					triX + 1, triZ + 1, heightmap->sizeX + 1);
				indices[index++] = INDEX(triX + 1, triZ, heightmap->sizeX + 1);
				indices[index++] = INDEX(triX, triZ, heightmap->sizeX + 1);
			}
		}

		// Upload the model to the GPU
		HeightmapModel hm = {};
		Mesh *m = &hm.mesh;

		glGenBuffers(1, &m->vertexBuffer);
		glGenVertexArrays(1, &m->vertexArray);

		uint32 bufferIndex = 0;

		glBindBuffer(GL_ARRAY_BUFFER, m->vertexBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			sizeof(Vertex) * numVerts,
			verts,
			GL_STATIC_DRAW);

		glBindVertexArray(m->vertexArray);
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

		glVertexAttribPointer(
			bufferIndex++,
			NUM_BONES,
			GL_UNSIGNED_INT,
			GL_FALSE,
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

		glGenBuffers(1, &m->indexBuffer);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->indexBuffer);
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			sizeof(uint32) * numIndices,
			indices,
			GL_STATIC_DRAW);

		m->numIndices = numIndices;

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		free(verts);
		free(indices);

		if (createHeightmapMaterial(
			idFromName(heightmap->materialName),
			&hm.material) == -1)
		{
			continue;
		}

		hashMapInsert(map, &entityID, &hm);
	}
}

extern real64 alpha;

internal
void beginRenderHeightmapSystem(Scene *scene, real64 dt)
{
	glUseProgram(shaderProgram);

	if (cameraSetUniforms(
		scene,
		viewUniform,
		projectionUniform) == -1)
	{
		return;
	}

	GLint textureIndex = 0;
	setMaterialUniform(&materialUniform, &textureIndex);
}

internal
void runRenderHeightmapSystem(Scene *scene, UUID entityID, real64 dt)
{
	if (!sceneGetComponentFromEntity(
		scene,
		scene->mainCamera,
		cameraComponentID))
	{
		return;
	}

	HashMap *sceneMap = (HashMap *)hashMapGetData(heightmapModels, &scene);
	ASSERT(sceneMap);

	HeightmapModel *heightmapModel = hashMapGetData(
		*sceneMap,
		&entityID);
	if (!heightmapModel)
	{
		LOG("No heightmap loaded for entity %s, skipping render\n",
			entityID.string);
		return;
	}

	Mesh *heightmap = &heightmapModel->mesh;
	Material *material = &heightmapModel->material;

	TransformComponent *transform = sceneGetComponentFromEntity(
		scene,
		entityID,
		transformComponentID);

	kmMat4 transformMat = tGetInterpolatedTransformMatrix(
		transform,
		alpha);

	if (setUniform(modelUniform, 1, &transformMat))
	{
		LOG("Unable to set model uniform\n");
		return;
	}

	glBindVertexArray(heightmap->vertexArray);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, heightmap->indexBuffer);

	for (uint8 i = 0; i < NUM_VERTEX_ATTRIBUTES; ++i)
	{
		glEnableVertexAttribArray(i);
	}

	GLint textureIndex = 0;
	activateMaterialTextures(material, &textureIndex);
	setMaterialValuesUniform(&materialValuesUniform, material);

	// Make this use a triangle list
	glDrawElements(
		GL_TRIANGLES,
		heightmap->numIndices,
		GL_UNSIGNED_INT,
		NULL);
	logGLError(false, "Error while drawing heightmap");

	for (uint8 i = 0; i < MATERIAL_COMPONENT_TYPE_COUNT; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	for (uint8 i = 0; i < NUM_VERTEX_ATTRIBUTES; i++)
	{
		glDisableVertexAttribArray(i);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

internal
void endRenderHeightmapSystem(Scene *scene, real64 dt)
{
	glUseProgram(0);
}

internal
void shutdownRenderHeightmapSystem(Scene *scene)
{
	HashMap *map = hashMapGetData(heightmapModels, &scene);

	for (ComponentDataTableIterator itr = cdtGetIterator(
			 *(ComponentDataTable **)hashMapGetData(
				 scene->componentTypes,
				 &heightmapComponentID));
		 !cdtIteratorAtEnd(itr);
		 cdtMoveIterator(&itr))
	{
		HeightmapComponent *heightmap = cdtIteratorGetData(itr);
		UUID entity = cdtIteratorGetUUID(itr);

		HeightmapModel *hm = hashMapGetData(*map, &entity);
		Mesh *m = &hm->mesh;

		glBindVertexArray(m->vertexArray);
		glDeleteBuffers(1, &m->vertexBuffer);
		glDeleteBuffers(1, &m->indexBuffer);
		glBindVertexArray(0);

		glDeleteVertexArrays(1, &m->vertexArray);

		// Delete the material used for this heightmap
		freeMaterial(&hm->material);

		free(dGeomGetData(heightmap->heightfieldGeom));
		dHeightfieldDataID heightfieldData = dGeomHeightfieldGetHeightfieldData(
			heightmap->heightfieldGeom);
		dGeomDestroy(heightmap->heightfieldGeom);
		dGeomHeightfieldDataDestroy(heightfieldData);
	}

	hashMapClear(*map);
	freeHashMap(map);
	hashMapDelete(heightmapModels, &scene);

	if (--rendererRefCount == 0)
	{
		LOG("Shutting down heightmap renderer...\n");

		glDeleteProgram(shaderProgram);
		freeHashMap(&heightmapModels);

		LOG("Successfully shut down heightmap renderer\n");
	}
}

System createRenderHeightmapSystem(void)
{
	heightmapComponentID = idFromName("heightmap");
	cameraComponentID = idFromName("camera");
	transformComponentID = idFromName("transform");

	System ret = {};

	ret.componentTypes = createList(sizeof(UUID));
	listPushFront(&ret.componentTypes, &transformComponentID);
	listPushFront(&ret.componentTypes, &heightmapComponentID);

	ret.init = &initRenderHeightmapSystem;
	ret.begin = &beginRenderHeightmapSystem;
	ret.run = &runRenderHeightmapSystem;
	ret.end = &endRenderHeightmapSystem;
	ret.shutdown = &shutdownRenderHeightmapSystem;

	return ret;
}

int32 createHeightmapMaterial(UUID name, Material *material)
{
	LOG("Loading material (%s)...\n", name.string);

	material->name = name;
	material->doubleSided = false;

	loadMaterialFolders(material->name);

	for (uint32 i = 0; i < MATERIAL_COMPONENT_TYPE_COUNT; i++)
	{
		MaterialComponent *materialComponent = &material->components[i];
		MaterialComponentType materialComponentType =
			(MaterialComponentType)i;

		if (loadMaterialComponentTexture(
			material->name,
			materialComponentType,
			&materialComponent->texture) == -1)
		{
			return -1;
		}

		kmVec3Fill(&materialComponent->value, 1.0f, 1.0f, 1.0f);
	}

	LOG("Successfully loaded material (%s)\n", name.string);

	return 0;
}