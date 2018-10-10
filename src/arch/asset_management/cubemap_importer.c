#include "asset_management/cubemap_importer.h"
#include "asset_management/model.h"
#include "asset_management/mesh.h"

#include "components/transform.h"

#include "core/config.h"
#include "core/log.h"

#include "renderer/shader.h"
#include "renderer/renderer_utilities.h"

#include <kazmath/mat4.h>

#define CUBEMAP_MESH_NAME "cubemap"

bool cubemapMeshLoaded;
Mesh cubemapMesh;

#define CUBEMAP_VERTEX_SHADER_FILE "resources/shaders/cubemap.vert"
#define CUBEMAP_CONVERTER_FRAGMENT_SHADER_FILE \
	"resources/shaders/convert_cubemap.frag"

typedef struct cubemap_converter_shader_t
{
	GLuint shaderProgram;
	Uniform cubemapTransformUniform;
	Uniform equirectangularCubemapTextureUniform;
} CubemapConverterShader;

internal CubemapConverterShader cubemapConverterShader;

internal GLuint cubemapFramebuffer;
internal GLuint cubemapRenderbuffer;

internal kmMat4 cubemapTransforms[6];

extern Config config;

internal int32 loadCubemapMesh(void);
internal void freeCubemapMesh(void);

internal void initializeCubemapConverterShader(void);
internal void convertCubemap(Cubemap *cubemap);

void initializeCubemapImporter(void)
{
	if (loadCubemapMesh() == -1)
	{
		cubemapMeshLoaded = false;
	}
	else
	{
		cubemapMeshLoaded = true;
	}

	initializeCubemapConverterShader();

	glGenRenderbuffers(1, &cubemapRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, cubemapRenderbuffer);

	glRenderbufferStorage(
		GL_RENDERBUFFER,
		GL_DEPTH_COMPONENT24,
		config.graphicsConfig.cubemapResolution,
		config.graphicsConfig.cubemapResolution);

	glGenFramebuffers(1, &cubemapFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, cubemapFramebuffer);

	glFramebufferRenderbuffer(
		GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER,
		cubemapRenderbuffer);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	kmMat4 cubemapProjection;
	kmMat4PerspectiveProjection(
		&cubemapProjection,
		90.0f,
		1.0f,
		0.1f,
		10.0f);

	kmQuaternion cubemapRotations[6];

	// Right Face
	kmQuaternionLookRotation(
		&cubemapRotations[0],
		&KM_VEC3_POS_X,
		&KM_VEC3_NEG_Y);

	// Left Face
	kmQuaternionLookRotation(
		&cubemapRotations[1],
		&KM_VEC3_NEG_X,
		&KM_VEC3_NEG_Y);

	// Top Face
	kmQuaternionLookRotation(
		&cubemapRotations[2],
		&KM_VEC3_POS_Y,
		&KM_VEC3_POS_Z);

	// Bottom Face
	kmQuaternionLookRotation(
		&cubemapRotations[3],
		&KM_VEC3_NEG_Y,
		&KM_VEC3_NEG_Z);

	// Front Face
	kmQuaternionLookRotation(
		&cubemapRotations[4],
		&KM_VEC3_POS_Z,
		&KM_VEC3_NEG_Y);

	// Back Face
	kmQuaternionLookRotation(
		&cubemapRotations[5],
		&KM_VEC3_NEG_Z,
		&KM_VEC3_NEG_Y);

	for (uint8 i = 0; i < 6; i++)
	{
		kmQuaternionInverse(&cubemapRotations[i], &cubemapRotations[i]);
	}

	kmVec3 cubemapScale;
	kmVec3Fill(&cubemapScale, 1.0f, 1.0f, 1.0f);

	for (uint8 i = 0; i < 6; i++)
	{
		kmMat4 cubemapView = tComposeMat4(
			&KM_VEC3_ZERO,
			&cubemapRotations[i],
			&cubemapScale);
		kmMat4Inverse(&cubemapView, &cubemapView);
		kmMat4Multiply(&cubemapTransforms[i], &cubemapProjection, &cubemapView);
	}
}

void importCubemap(Cubemap *cubemap)
{
	if (!cubemapMeshLoaded)
	{
		return;
	}

	convertCubemap(cubemap);
}

void shutdownCubemapImporter(void)
{
	glDeleteProgram(cubemapConverterShader.shaderProgram);

	glDeleteFramebuffers(1, &cubemapFramebuffer);
	glDeleteRenderbuffers(1, &cubemapRenderbuffer);

	freeCubemapMesh();
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

void initializeCubemapConverterShader(void)
{
	createShaderProgram(
		CUBEMAP_VERTEX_SHADER_FILE,
		NULL,
		NULL,
		NULL,
		CUBEMAP_CONVERTER_FRAGMENT_SHADER_FILE,
		NULL,
		&cubemapConverterShader.shaderProgram);

	getUniform(
		cubemapConverterShader.shaderProgram,
		"cubemapTransform",
		UNIFORM_MAT4,
		&cubemapConverterShader.cubemapTransformUniform);
	getUniform(
		cubemapConverterShader.shaderProgram,
		"equirectangularCubemapTexture",
		UNIFORM_TEXTURE_2D,
		&cubemapConverterShader.equirectangularCubemapTextureUniform);
}

void convertCubemap(Cubemap *cubemap)
{
	glViewport(
		0,
		0,
		config.graphicsConfig.cubemapResolution,
		config.graphicsConfig.cubemapResolution);

	glBindFramebuffer(GL_FRAMEBUFFER, cubemapFramebuffer);
	glUseProgram(cubemapConverterShader.shaderProgram);

	uint32 textureIndex = 0;
	setUniform(
		cubemapConverterShader.equirectangularCubemapTextureUniform,
		1,
		&textureIndex);

	glBindVertexArray(cubemapMesh.vertexArray);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubemapMesh.indexBuffer);

	for (uint8 j = 0; j < NUM_VERTEX_ATTRIBUTES; j++)
	{
		glEnableVertexAttribArray(j);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cubemap->equirectangularID);

	glFrontFace(GL_CW);

	for (uint8 i = 0; i < 6; i++)
	{
		glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			cubemap->cubemapID,
			0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		setUniform(
			cubemapConverterShader.cubemapTransformUniform,
			1,
			&cubemapTransforms[i]);

		glDrawElements(
			GL_TRIANGLES,
			cubemapMesh.numIndices,
			GL_UNSIGNED_INT,
			NULL);

		logGLError(
			false,
			"Failed to convert cubemap (%s)",
			cubemap->name.string);
	}

	glFrontFace(GL_CCW);

	glBindTexture(GL_TEXTURE_2D, 0);

	for (uint8 j = 0; j < NUM_VERTEX_ATTRIBUTES; j++)
	{
		glDisableVertexAttribArray(j);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteTextures(1, &cubemap->equirectangularID);
	cubemap->equirectangularID = 0;
}