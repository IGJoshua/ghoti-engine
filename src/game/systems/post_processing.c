#include "defines.h"

#include "core/log.h"
#include "core/config.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "renderer/renderer_types.h"
#include "renderer/renderer_utilities.h"
#include "renderer/shader.h"

#define VERTEX_SHADER_FILE "resources/shaders/quad.vert"
#define DEFAULT_FRAGMENT_SHADER_FILE \
	"resources/shaders/default_post_process.frag"
#define GRAYSCALE_FRAGMENT_SHADER_FILE \
	"resources/shaders/grayscale_post_process.frag"

internal GLuint shaderProgram;

internal Uniform screenTextureUniform;

GLuint screenFramebufferMSAA;
GLuint screenFramebuffer;

internal GLuint screenTextureMSAA;
internal GLuint screenTexture;

internal GLuint screenRenderbuffer;

#define NUM_QUAD_VERTEX_ATTRIBUTES 2

typedef struct quad_vertex_t
{
	kmVec2 position;
	kmVec2 uv;
} QuadVertex;

internal QuadVertex quadVertices[4];

internal GLuint quadVertexBuffer;
internal GLuint quadVertexArray;

extern Config config;

uint32 postProcessingSystemRefCount = 0;

internal int32 previousViewportWidth = 0;
internal int32 previousViewportHeight = 0;

extern int32 viewportWidth;
extern int32 viewportHeight;

internal void freeFramebuffers(void);
internal void createFramebuffers(void);

internal void initPostProcessingSystem(Scene *scene)
{
	if (postProcessingSystemRefCount == 0)
	{
		LOG("Initializing post processing system...\n");

		createShaderProgram(
			VERTEX_SHADER_FILE,
			NULL,
			NULL,
			NULL,
			config.graphicsConfig.grayscalePostProcess ?
				GRAYSCALE_FRAGMENT_SHADER_FILE : DEFAULT_FRAGMENT_SHADER_FILE,
			NULL,
			&shaderProgram);

		getUniform(
			shaderProgram,
			"screenTexture",
			UNIFORM_TEXTURE_2D,
			&screenTextureUniform);

		createFramebuffers();

		kmVec2Fill(&quadVertices[0].position, -1.0f, 1.0f);
		kmVec2Fill(&quadVertices[0].uv, 0.0f, 1.0f);
		kmVec2Fill(&quadVertices[1].position, -1.0f, -1.0f);
		kmVec2Fill(&quadVertices[1].uv, 0.0f, 0.0f);
		kmVec2Fill(&quadVertices[2].position, 1.0f, 1.0f);
		kmVec2Fill(&quadVertices[2].uv, 1.0f, 1.0f);
		kmVec2Fill(&quadVertices[3].position, 1.0f, -1.0f);
		kmVec2Fill(&quadVertices[3].uv, 1.0f, 0.0f);

		glGenBuffers(1, &quadVertexBuffer);
		glGenVertexArrays(1, &quadVertexArray);

		uint32 bufferIndex = 0;

		glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			sizeof(QuadVertex) * 4,
			quadVertices,
			GL_STATIC_DRAW);

		glBindVertexArray(quadVertexArray);
		glVertexAttribPointer(
			bufferIndex++,
			2,
			GL_FLOAT,
			GL_FALSE,
			sizeof(QuadVertex),
			(GLvoid*)offsetof(QuadVertex, position));

		glVertexAttribPointer(
			bufferIndex++,
			2,
			GL_FLOAT,
			GL_FALSE,
			sizeof(QuadVertex),
			(GLvoid*)offsetof(QuadVertex, uv));

		LOG("Successfully initialized post processing system\n");
	}

	postProcessingSystemRefCount++;
}

internal void beginPostProcessingSystem(Scene *scene, real64 dt)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, screenFramebufferMSAA);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenFramebuffer);

	glBlitFramebuffer(
		0,
		0,
		viewportWidth,
		viewportHeight,
		0,
		0,
		viewportWidth,
		viewportHeight,
		GL_COLOR_BUFFER_BIT,
		GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);

	if (viewportWidth != previousViewportWidth ||
		viewportHeight != previousViewportHeight)
	{
		freeFramebuffers();
		createFramebuffers();
	}

	previousViewportWidth = viewportWidth;
	previousViewportHeight = viewportHeight;

	glUseProgram(shaderProgram);

	uint32 textureIndex = 0;
	setUniform(screenTextureUniform, 1, &textureIndex);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, screenTexture);

	glBindVertexArray(quadVertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);

	for (uint8 i = 0; i < NUM_QUAD_VERTEX_ATTRIBUTES; i++)
	{
		glEnableVertexAttribArray(i);
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	logGLError(false, "Failed to draw post processing quad");
}

internal void endPostProcessingSystem(Scene *scene, real64 dt)
{
	for (uint8 i = 0; i < NUM_QUAD_VERTEX_ATTRIBUTES; i++)
	{
		glDisableVertexAttribArray(i);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}

internal void shutdownPostProcessingSystem(Scene *scene)
{
	if (--postProcessingSystemRefCount == 0)
	{
		LOG("Shutting down post processing system...\n");

		glDeleteProgram(shaderProgram);

		freeFramebuffers();

		glBindVertexArray(quadVertexArray);
		glDeleteBuffers(1, &quadVertexBuffer);
		glBindVertexArray(0);

		glDeleteVertexArrays(1, &quadVertexArray);

		LOG("Successfully shut down post processing system\n");
	}
}

System createPostProcessingSystem(void)
{
	System system = {};

	system.componentTypes = createList(sizeof(UUID));

	system.init = &initPostProcessingSystem;
	system.begin = &beginPostProcessingSystem;
	system.end = &endPostProcessingSystem;
	system.shutdown = &shutdownPostProcessingSystem;

	return system;
}

void freeFramebuffers(void)
{
	glDeleteFramebuffers(1, &screenFramebufferMSAA);
	glDeleteFramebuffers(1, &screenFramebuffer);

	glDeleteTextures(1, &screenTextureMSAA);
	glDeleteTextures(1, &screenTexture);

	glDeleteRenderbuffers(1, &screenRenderbuffer);
}

void createFramebuffers(void)
{
	glGenTextures(1, &screenTextureMSAA);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, screenTextureMSAA);

	glTexImage2DMultisample(
		GL_TEXTURE_2D_MULTISAMPLE,
		config.graphicsConfig.numMSAASamples,
		GL_RGB,
		viewportWidth,
		viewportHeight,
		GL_TRUE);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	glGenFramebuffers(1, &screenFramebufferMSAA);
	glBindFramebuffer(GL_FRAMEBUFFER, screenFramebufferMSAA);

	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D_MULTISAMPLE,
		screenTextureMSAA,
		0);

	glGenRenderbuffers(1, &screenRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, screenRenderbuffer);

	glRenderbufferStorageMultisample(
		GL_RENDERBUFFER,
		config.graphicsConfig.numMSAASamples,
		GL_DEPTH24_STENCIL8,
		viewportWidth,
		viewportHeight);

	glFramebufferRenderbuffer(
		GL_FRAMEBUFFER,
		GL_DEPTH_STENCIL_ATTACHMENT,
		GL_RENDERBUFFER,
		screenRenderbuffer);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenTextures(1, &screenTexture);
	glBindTexture(GL_TEXTURE_2D, screenTexture);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGB,
		viewportWidth,
		viewportHeight,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenFramebuffers(1, &screenFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, screenFramebuffer);

	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		screenTexture,
		0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}