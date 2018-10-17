#include "asset_management/cubemap_importer.h"
#include "asset_management/model.h"
#include "asset_management/mesh.h"
#include "asset_management/texture.h"

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

#define CUBEMAP_CONVOLUTION_FRAGMENT_SHADER_FILE \
	"resources/shaders/convolute_cubemap.frag"

typedef struct cubemap_convolution_shader_t
{
	GLuint shaderProgram;
	Uniform cubemapTransformUniform;
	Uniform cubemapTextureUniform;
} CubemapConvolutionShader;

#define CUBEMAP_PREFILTER_FRAGMENT_SHADER_FILE \
	"resources/shaders/prefilter_cubemap.frag"

typedef struct cubemap_prefilter_shader_t
{
	GLuint shaderProgram;
	Uniform cubemapTransformUniform;
	Uniform cubemapTextureUniform;
	Uniform cubemapResolutionUniform;
	Uniform roughnessUniform;
} CubemapPrefilterShader;

internal CubemapConverterShader cubemapConverterShader;
internal CubemapConvolutionShader cubemapConvolutionShader;
internal CubemapPrefilterShader cubemapPrefilterShader;

internal GLuint cubemapFramebuffer;
internal GLuint cubemapRenderbuffer;

internal kmMat4 cubemapTransforms[6];

#define BRDF_LUT_FILE "brdf_lut"

GLuint brdfLUT;

extern Config config;

internal int32 loadCubemapMesh(void);
internal void freeCubemapMesh(void);

internal int32 loadBRDFLUT(void);

internal void createCubemapTextures(Cubemap *cubemap);

internal void initializeCubemapConverterShader(void);
internal void convertCubemap(Cubemap *cubemap);

internal void initializeCubemapConvolutionShader(void);
internal void convoluteCubemap(Cubemap *cubemap);

internal void initializeCubemapPrefilterShader(void);
internal void prefilterCubemap(Cubemap *cubemap);

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
	initializeCubemapConvolutionShader();
	initializeCubemapPrefilterShader();

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

	loadBRDFLUT();

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

	createCubemapTextures(cubemap);
	convertCubemap(cubemap);
	convoluteCubemap(cubemap);
	prefilterCubemap(cubemap);
}

void shutdownCubemapImporter(void)
{
	glDeleteProgram(cubemapConverterShader.shaderProgram);
	glDeleteProgram(cubemapConvolutionShader.shaderProgram);
	glDeleteProgram(cubemapPrefilterShader.shaderProgram);

	glDeleteFramebuffers(1, &cubemapFramebuffer);
	glDeleteRenderbuffers(1, &cubemapRenderbuffer);

	glDeleteTextures(1, &brdfLUT);

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

int32 loadBRDFLUT(void)
{
	int32 error = 0;

	char *filename = getFullFilePath(
		BRDF_LUT_FILE,
		"hdr",
		"resources/cubemaps");

	HDRTextureData data;
	error = loadHDRTextureData(
		ASSET_LOG_TYPE_NONE,
		"BRDF LUT",
		NULL,
		filename,
		3,
		false,
		&data);

	free(filename);

	if (error != - 1)
	{
		glGenTextures(1, &brdfLUT);
		glBindTexture(GL_TEXTURE_2D, brdfLUT);

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RG16F,
			data.width,
			data.height,
			0,
			GL_RGB,
			GL_FLOAT,
			data.data);

		free(data.data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	return error;
}

void createCubemapTextures(Cubemap *cubemap)
{
	glGenTextures(1, &cubemap->cubemapID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->cubemapID);

	for (uint8 i = 0; i < 6; i++)
	{
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGB16F,
			config.graphicsConfig.cubemapResolution,
			config.graphicsConfig.cubemapResolution,
			0,
			GL_RGB,
			GL_FLOAT,
			NULL);
	}

	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_MAG_FILTER,
		GL_LINEAR);
	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_WRAP_S,
		GL_CLAMP_TO_EDGE);
	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_WRAP_T,
		GL_CLAMP_TO_EDGE);
	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_WRAP_R,
		GL_CLAMP_TO_EDGE);

	glGenTextures(1, &cubemap->irradianceID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->irradianceID);

	for (uint8 i = 0; i < 6; i++)
	{
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGB16F,
			config.graphicsConfig.irradianceMapResolution,
			config.graphicsConfig.irradianceMapResolution,
			0,
			GL_RGB,
			GL_FLOAT,
			NULL);
	}

	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_MIN_FILTER,
		GL_LINEAR);
	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_MAG_FILTER,
		GL_LINEAR);
	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_WRAP_S,
		GL_CLAMP_TO_EDGE);
	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_WRAP_T,
		GL_CLAMP_TO_EDGE);
	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_WRAP_R,
		GL_CLAMP_TO_EDGE);

	glGenTextures(1, &cubemap->prefilterID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->prefilterID);

	for (uint8 i = 0; i < 6; i++)
	{
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGB16F,
			config.graphicsConfig.prefilterMapResolution,
			config.graphicsConfig.prefilterMapResolution,
			0,
			GL_RGB,
			GL_FLOAT,
			NULL);
	}

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
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

	glBindRenderbuffer(GL_RENDERBUFFER, cubemapRenderbuffer);
	glRenderbufferStorage(
		GL_RENDERBUFFER,
		GL_DEPTH_COMPONENT24,
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

	for (uint8 i = 0; i < NUM_VERTEX_ATTRIBUTES; i++)
	{
		glEnableVertexAttribArray(i);
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

	for (uint8 i = 0; i < NUM_VERTEX_ATTRIBUTES; i++)
	{
		glDisableVertexAttribArray(i);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glDeleteTextures(1, &cubemap->equirectangularID);
	cubemap->equirectangularID = 0;

	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->cubemapID);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void initializeCubemapConvolutionShader(void)
{
	createShaderProgram(
		CUBEMAP_VERTEX_SHADER_FILE,
		NULL,
		NULL,
		NULL,
		CUBEMAP_CONVOLUTION_FRAGMENT_SHADER_FILE,
		NULL,
		&cubemapConvolutionShader.shaderProgram);

	getUniform(
		cubemapConvolutionShader.shaderProgram,
		"cubemapTransform",
		UNIFORM_MAT4,
		&cubemapConvolutionShader.cubemapTransformUniform);
	getUniform(
		cubemapConvolutionShader.shaderProgram,
		"cubemapTexture",
		UNIFORM_TEXTURE_CUBE_MAP,
		&cubemapConvolutionShader.cubemapTextureUniform);
}

void convoluteCubemap(Cubemap *cubemap)
{
	glViewport(
		0,
		0,
		config.graphicsConfig.irradianceMapResolution,
		config.graphicsConfig.irradianceMapResolution);

	glBindRenderbuffer(GL_RENDERBUFFER, cubemapRenderbuffer);
	glRenderbufferStorage(
		GL_RENDERBUFFER,
		GL_DEPTH_COMPONENT24,
		config.graphicsConfig.irradianceMapResolution,
		config.graphicsConfig.irradianceMapResolution);

	glBindFramebuffer(GL_FRAMEBUFFER, cubemapFramebuffer);
	glUseProgram(cubemapConvolutionShader.shaderProgram);

	uint32 textureIndex = 0;
	setUniform(
		cubemapConvolutionShader.cubemapTextureUniform,
		1,
		&textureIndex);

	glBindVertexArray(cubemapMesh.vertexArray);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubemapMesh.indexBuffer);

	for (uint8 i = 0; i < NUM_VERTEX_ATTRIBUTES; i++)
	{
		glEnableVertexAttribArray(i);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->cubemapID);

	glFrontFace(GL_CW);

	for (uint8 i = 0; i < 6; i++)
	{
		glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			cubemap->irradianceID,
			0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		setUniform(
			cubemapConvolutionShader.cubemapTransformUniform,
			1,
			&cubemapTransforms[i]);

		glDrawElements(
			GL_TRIANGLES,
			cubemapMesh.numIndices,
			GL_UNSIGNED_INT,
			NULL);

		logGLError(
			false,
			"Failed to convolute cubemap (%s)",
			cubemap->name.string);
	}

	glFrontFace(GL_CCW);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	for (uint8 i = 0; i < NUM_VERTEX_ATTRIBUTES; i++)
	{
		glDisableVertexAttribArray(i);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void initializeCubemapPrefilterShader(void)
{
	createShaderProgram(
		CUBEMAP_VERTEX_SHADER_FILE,
		NULL,
		NULL,
		NULL,
		CUBEMAP_PREFILTER_FRAGMENT_SHADER_FILE,
		NULL,
		&cubemapPrefilterShader.shaderProgram);

	getUniform(
		cubemapPrefilterShader.shaderProgram,
		"cubemapTransform",
		UNIFORM_MAT4,
		&cubemapPrefilterShader.cubemapTransformUniform);
	getUniform(
		cubemapPrefilterShader.shaderProgram,
		"cubemapTexture",
		UNIFORM_TEXTURE_CUBE_MAP,
		&cubemapPrefilterShader.cubemapTextureUniform);
	getUniform(
		cubemapPrefilterShader.shaderProgram,
		"cubemapResolution",
		UNIFORM_FLOAT,
		&cubemapPrefilterShader.cubemapResolutionUniform);
	getUniform(
		cubemapPrefilterShader.shaderProgram,
		"roughness",
		UNIFORM_FLOAT,
		&cubemapPrefilterShader.roughnessUniform);
}

void prefilterCubemap(Cubemap *cubemap)
{
	glBindFramebuffer(GL_FRAMEBUFFER, cubemapFramebuffer);
	glUseProgram(cubemapPrefilterShader.shaderProgram);

	uint32 textureIndex = 0;
	setUniform(
		cubemapPrefilterShader.cubemapTextureUniform,
		1,
		&textureIndex);

	setUniform(
		cubemapPrefilterShader.cubemapResolutionUniform,
		1,
		&config.graphicsConfig.cubemapResolution);

	glBindVertexArray(cubemapMesh.vertexArray);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubemapMesh.indexBuffer);

	for (uint8 i = 0; i < NUM_VERTEX_ATTRIBUTES; i++)
	{
		glEnableVertexAttribArray(i);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->cubemapID);

	glFrontFace(GL_CW);

	const uint32 maxNumMipLevels = 5;
	for (uint8 mipLevel = 0; mipLevel < maxNumMipLevels; mipLevel++)
	{
		uint32 mipLevelSize =
			config.graphicsConfig.prefilterMapResolution * powf(0.5f, mipLevel);

		glViewport(0, 0, mipLevelSize, mipLevelSize);
		glBindRenderbuffer(GL_RENDERBUFFER, cubemapRenderbuffer);
		glRenderbufferStorage(
			GL_RENDERBUFFER,
			GL_DEPTH_COMPONENT24,
			mipLevelSize,
			mipLevelSize);

		real32 roughness = (real32)mipLevel / (real32)(maxNumMipLevels - 1);
		setUniform(cubemapPrefilterShader.roughnessUniform, 1, &roughness);

		for (uint8 i = 0; i < 6; i++)
		{
			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				cubemap->prefilterID,
				mipLevel);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			setUniform(
				cubemapPrefilterShader.cubemapTransformUniform,
				1,
				&cubemapTransforms[i]);

			glDrawElements(
				GL_TRIANGLES,
				cubemapMesh.numIndices,
				GL_UNSIGNED_INT,
				NULL);

			logGLError(
				false,
				"Failed to prefilter cubemap (%s)",
				cubemap->name.string);
		}
	}

	glFrontFace(GL_CCW);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	for (uint8 i = 0; i < NUM_VERTEX_ATTRIBUTES; i++)
	{
		glDisableVertexAttribArray(i);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->cubemapID);
	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_MIN_FILTER,
		GL_LINEAR);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}