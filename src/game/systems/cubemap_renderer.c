#include "defines.h"

#include "core/log.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/cubemap.h"
#include "asset_management/model.h"
#include "asset_management/mesh.h"

#include "renderer/renderer_types.h"
#include "renderer/renderer_utilities.h"
#include "renderer/shader.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "components/component_types.h"
#include "components/transform.h"
#include "components/camera.h"

#include "data/data_types.h"
#include "data/list.h"

#define VERTEX_SHADER_FILE "resources/shaders/cubemap.vert"
#define FRAGMENT_SHADER_FILE "resources/shaders/cubemap.frag"

internal GLuint shaderProgram;

internal Uniform modelUniform;
internal Uniform viewUniform;
internal Uniform projectionUniform;

internal Uniform cubemapTextureUniform;

internal uint32 cubemapRendererRefCount = 0;

internal UUID transformComponentID = {};
internal UUID cameraComponentID = {};
internal UUID cubemapComponentID = {};

internal CameraComponent *camera;
internal TransformComponent *cameraTransform;

internal GLboolean glDepthMaskValue;

Cubemap currentCubemap = {};

extern real64 alpha;

#define CUBEMAP_MESH_NAME "cubemap"

internal bool cubemapMeshLoaded;
internal Mesh cubemapMesh;

internal int32 loadCubemapMesh(void);
internal void freeCubemapMesh(void);

internal void initCubemapRendererSystem(Scene *scene)
{
	if (cubemapRendererRefCount == 0)
	{
		LOG("Initializing cubemap renderer...\n");

		cubemapMeshLoaded = loadCubemapMesh() != -1;

		createShaderProgram(
			VERTEX_SHADER_FILE,
			NULL,
			NULL,
			NULL,
			FRAGMENT_SHADER_FILE,
			NULL,
			&shaderProgram);

		getUniform(shaderProgram, "model", UNIFORM_MAT4, &modelUniform);
		getUniform(shaderProgram, "view", UNIFORM_MAT4, &viewUniform);
		getUniform(
			shaderProgram,
			"projection",
			UNIFORM_MAT4,
			&projectionUniform);

		getUniform(
			shaderProgram,
			"cubemapTexture",
			UNIFORM_TEXTURE_CUBE_MAP,
			&cubemapTextureUniform);

		LOG("Successfully initialized cubemap renderer\n");
	}

	cubemapRendererRefCount++;
}

internal void beginCubemapRendererSystem(Scene *scene, real64 dt)
{
	camera = sceneGetComponentFromEntity(
		scene,
		scene->mainCamera,
		cameraComponentID);

	cameraTransform = sceneGetComponentFromEntity(
		scene,
		scene->mainCamera,
		transformComponentID);

	if (!camera || !cameraTransform || !cubemapMeshLoaded)
	{
		return;
	}

	memset(&currentCubemap, 0, sizeof(Cubemap));

	glUseProgram(shaderProgram);

	TransformComponent cubemapCameraTransform = *cameraTransform;
	kmVec3Zero(&cubemapCameraTransform.lastGlobalPosition);
	kmVec3Zero(&cubemapCameraTransform.globalPosition);

	cameraSetUniforms(
		camera,
		&cubemapCameraTransform,
		viewUniform,
		projectionUniform);

	kmMat4 worldMatrix;
	kmMat4Scaling(&worldMatrix, 2.0f, 2.0f, 2.0f);

	setUniform(modelUniform, 1, &worldMatrix);

	GLint textureIndex = 0;
	setUniform(cubemapTextureUniform, 1, &textureIndex);
}

internal void runCubemapRendererSystem(Scene *scene, UUID entity, real64 dt)
{
	if (!camera ||
		!cameraTransform ||
		!cubemapMeshLoaded ||
		strlen(currentCubemap.name.string) > 0)
	{
		return;
	}

	CubemapComponent *cubemapComponent = sceneGetComponentFromEntity(
		scene,
		entity,
		cubemapComponentID);

	Cubemap cubemap = getCubemap(cubemapComponent->name);

	if (strlen(cubemap.name.string) == 0)
	{
		return;
	}

	currentCubemap = cubemap;

	glBindVertexArray(cubemapMesh.vertexArray);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubemapMesh.indexBuffer);

	for (uint8 j = 0; j < NUM_VERTEX_ATTRIBUTES; j++)
	{
		glEnableVertexAttribArray(j);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.cubemapID);

	glGetBooleanv(GL_DEPTH_WRITEMASK, &glDepthMaskValue);

	glDepthMask(GL_FALSE);
	glFrontFace(GL_CW);

	glDrawElements(
		GL_TRIANGLES,
		cubemapMesh.numIndices,
		GL_UNSIGNED_INT,
		NULL);

	logGLError(false, "Failed to draw cubemap (%s)", cubemap.name.string);

	glDepthMask(glDepthMaskValue);
	glFrontFace(GL_CCW);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	for (uint8 j = 0; j < NUM_VERTEX_ATTRIBUTES; j++)
	{
		glDisableVertexAttribArray(j);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

internal void endCubemapRendererSystem(Scene *scene, real64 dt)
{
	if (!camera || !cameraTransform || !cubemapMeshLoaded)
	{
		return;
	}

	glUseProgram(0);
}

internal void shutdownCubemapRendererSystem(Scene *scene)
{
	if (--cubemapRendererRefCount == 0)
	{
		LOG("Shutting down cubemap renderer...\n");

		glDeleteProgram(shaderProgram);

		freeCubemapMesh();

		LOG("Successfully shut down cubemap renderer\n");
	}
}

System createCubemapRendererSystem(void)
{
	System system = {};

	transformComponentID = idFromName("transform");
	cameraComponentID = idFromName("camera");
	cubemapComponentID = idFromName("cubemap");

	system.componentTypes = createList(sizeof(UUID));
	listPushFront(&system.componentTypes, &cubemapComponentID);

	system.init = &initCubemapRendererSystem;
	system.begin = &beginCubemapRendererSystem;
	system.run = &runCubemapRendererSystem;
	system.end = &endCubemapRendererSystem;
	system.shutdown = &shutdownCubemapRendererSystem;

	return system;
}

int32 loadCubemapMesh(void)
{
	int32 error = 0;

	LOG("Loading model (%s)...\n", CUBEMAP_MESH_NAME);

	char *cubemapMeshFolder = getFullFilePath(
		CUBEMAP_MESH_NAME,
		NULL,
		"resources/models");

	char *filename = getFullFilePath(
		CUBEMAP_MESH_NAME,
		"mesh",
		cubemapMeshFolder);

	free(cubemapMeshFolder);

	FILE *file = fopen(filename, "rb");

	if (file)
	{
		uint8 meshBinaryFileVersion;
		fread(&meshBinaryFileVersion, sizeof(uint8), 1, file);

		if (meshBinaryFileVersion < MESH_BINARY_FILE_VERSION)
		{
			LOG("WARNING: %s out of date\n", filename);
			error = -1;
		}
	}
	else
	{
		error = -1;
	}

	if (error != -1)
	{
		loadMesh(&cubemapMesh, file, NULL);
		uploadMeshToGPU(&cubemapMesh, CUBEMAP_MESH_NAME);

		fclose(file);
	}
	else
	{
		LOG("Failed to open %s\n", filename);
		error = -1;
	}

	free(filename);

	if (error != -1)
	{
		LOG("Successfully loaded model (%s)\n", CUBEMAP_MESH_NAME);
	}
	else
	{
		LOG("Failed to load model (%s)\n", CUBEMAP_MESH_NAME);
	}

	return error;
}

void freeCubemapMesh(void)
{
	LOG("Freeing model (%s)...\n", CUBEMAP_MESH_NAME);

	freeMesh(&cubemapMesh);

	LOG("Successfully freed model (%s)\n", CUBEMAP_MESH_NAME);
}