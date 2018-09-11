#include "defines.h"

#include "core/log.h"

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

#include <stddef.h>

internal uint32 debugRendererRefCount = 0;

#define NUM_DEBUG_VERTEX_ATTRIBUTES 2

typedef struct debug_vertex_t
{
	kmVec3 position;
	kmVec3 color;
} DebugVertex;

#define MAX_LINE_COUNT 4096
#define MAX_DEBUG_VERTEX_COUNT MAX_LINE_COUNT * 2

typedef struct debug_vertex_buffer_t
{
	DebugVertex vertices[MAX_DEBUG_VERTEX_COUNT];
	real32 sizes[MAX_DEBUG_VERTEX_COUNT];
	uint32 numVertices;
	GLuint vertexBuffer;
	GLuint vertexArray;
} DebugVertexBuffer;

DebugVertexBuffer pointVertexBuffer;
DebugVertexBuffer lineVertexBuffer;

internal GLuint shaderProgram;

internal Uniform viewUniform;
internal Uniform projectionUniform;

internal UUID transformComponentID = {};
internal UUID cameraComponentID = {};
internal UUID debugPrimitiveComponentID = {};

extern real64 alpha;

internal void initializeVertexBuffer(DebugVertexBuffer *vertexBuffer);

internal void clearVertices(DebugVertexBuffer *vertexBuffer);
internal void addVertex(
	DebugVertexBuffer *vertexBuffer,
	DebugVertex vertex,
	real32 size);

internal void addPoint(
	const kmVec3 *position,
	const kmVec3 *color,
	real32 size);
internal void addLine(
	const kmVec3 *positionA,
	const kmVec3 *positionB,
	const kmVec3 *colorA,
	const kmVec3 *colorB,
	real32 size);

internal void drawPrimitives(
	DebugVertexBuffer *vertexBuffer,
	GLenum primitiveType,
	const char *name);

internal void initDebugRendererSystem(Scene *scene)
{
	if (debugRendererRefCount == 0)
	{
		LOG("Initializing debug renderer...\n");

		createShaderProgram(
			"resources/shaders/debug.vert",
			NULL,
			NULL,
			NULL,
			"resources/shaders/debug.frag",
			NULL,
			&shaderProgram);

		initializeVertexBuffer(&pointVertexBuffer);
		initializeVertexBuffer(&lineVertexBuffer);

		getUniform(shaderProgram, "view", UNIFORM_MAT4, &viewUniform);
		getUniform(
			shaderProgram,
			"projection",
			UNIFORM_MAT4,
			&projectionUniform);

		LOG("Successfully initialized debug renderer\n");
	}

	debugRendererRefCount++;
}

internal void runDebugRendererSystem(Scene *scene, UUID entity, real64 dt)
{
	TransformComponent *transform = sceneGetComponentFromEntity(
		scene,
		entity,
		transformComponentID);
	DebugPrimitiveComponent *debugPrimitive = sceneGetComponentFromEntity(
		scene,
		entity,
		debugPrimitiveComponentID);

	kmVec3 position;
	tGetInterpolatedTransform(transform, &position, NULL, NULL, alpha);

	TransformComponent *endpointTransform = NULL;

	if (!debugPrimitive->visible)
	{
		return;
	}

	switch (debugPrimitive->type)
	{
		case DEBUG_PRIMITIVE_TYPE_POINT:
			addPoint(&position, &debugPrimitive->color, debugPrimitive->size);
			break;
		case DEBUG_PRIMITIVE_TYPE_LINE:
			endpointTransform =  sceneGetComponentFromEntity(
				scene,
				debugPrimitive->endpoint,
				transformComponentID);

			if (endpointTransform)
			{
				kmVec3 endpoint;
				tGetInterpolatedTransform(
					endpointTransform,
					&endpoint,
					NULL,
					NULL,
					alpha);

				addLine(
					&position,
					&endpoint,
					&debugPrimitive->color,
					&debugPrimitive->endpointColor,
					debugPrimitive->size);
			}

			break;
		case DEBUG_PRIMITIVE_TYPE_TRANSFORM:
			break;
		default:
			break;
	}
}

internal void endDebugRendererSystem(Scene *scene, real64 dt)
{
	glUseProgram(shaderProgram);

	if (cameraSetUniforms(
		scene,
		viewUniform,
		projectionUniform) == -1)
	{
		return;
	}

	drawPrimitives(&pointVertexBuffer, GL_POINTS, "point");
	drawPrimitives(&lineVertexBuffer, GL_LINES, "line");

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(0);
}

internal void shutdownDebugRendererSystem(Scene *scene)
{
	if (--debugRendererRefCount == 0)
	{
		LOG("Shutting down debug renderer...\n");

		glBindVertexArray(pointVertexBuffer.vertexArray);
		glDeleteBuffers(1, &pointVertexBuffer.vertexBuffer);
		glBindVertexArray(lineVertexBuffer.vertexArray);
		glDeleteBuffers(1, &lineVertexBuffer.vertexBuffer);
		glBindVertexArray(0);

		glDeleteVertexArrays(1, &pointVertexBuffer.vertexArray);
		glDeleteVertexArrays(1, &lineVertexBuffer.vertexArray);

		glDeleteProgram(shaderProgram);

		LOG("Successfully shut down debug renderer\n");
	}
}

System createDebugRendererSystem(void)
{
	System system = {};

	transformComponentID = idFromName("transform");
	cameraComponentID = idFromName("camera");
	debugPrimitiveComponentID = idFromName("debug_primitive");

	system.componentTypes = createList(sizeof(UUID));
	listPushFront(&system.componentTypes, &transformComponentID);
	listPushFront(&system.componentTypes, &debugPrimitiveComponentID);

	system.init = &initDebugRendererSystem;
	system.run = &runDebugRendererSystem;
	system.end = &endDebugRendererSystem;
	system.shutdown = &shutdownDebugRendererSystem;

	return system;
}

void initializeVertexBuffer(DebugVertexBuffer *vertexBuffer)
{
	glGenBuffers(1, &vertexBuffer->vertexBuffer);
	glGenVertexArrays(1, &vertexBuffer->vertexArray);

	uint32 bufferIndex = 0;

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vertexBuffer);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(DebugVertex) * MAX_DEBUG_VERTEX_COUNT,
		NULL,
		GL_DYNAMIC_DRAW);

	glBindVertexArray(vertexBuffer->vertexArray);
	glVertexAttribPointer(
		bufferIndex++,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(DebugVertex),
		(GLvoid*)offsetof(DebugVertex, position));

	glVertexAttribPointer(
		bufferIndex++,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(DebugVertex),
		(GLvoid*)offsetof(DebugVertex, color));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	clearVertices(vertexBuffer);
}

void clearVertices(DebugVertexBuffer *vertexBuffer)
{
	memset(
		vertexBuffer->vertices,
		0,
		sizeof(DebugVertex) * MAX_DEBUG_VERTEX_COUNT);
	memset(
		vertexBuffer->sizes,
		0,
		sizeof(real32) * MAX_DEBUG_VERTEX_COUNT);
	vertexBuffer->numVertices = 0;
}

void addVertex(DebugVertexBuffer *vertexBuffer, DebugVertex vertex, real32 size)
{
	if (vertexBuffer->numVertices + 1 < MAX_DEBUG_VERTEX_COUNT)
	{
		vertexBuffer->vertices[vertexBuffer->numVertices++] = vertex;
		vertexBuffer->sizes[vertexBuffer->numVertices] = size;
	}
}

void addPoint(const kmVec3 *position, const kmVec3 *color, real32 size)
{
	DebugVertex vertex;
	memset(&vertex, 0, sizeof(DebugVertex));

	kmVec3Assign(&vertex.position, position);
	kmVec3Assign(&vertex.color, color);
	addVertex(&pointVertexBuffer, vertex, size);
}

void addLine(
	const kmVec3 *positionA,
	const kmVec3 *positionB,
	const kmVec3 *colorA,
	const kmVec3 *colorB,
	real32 size)
{
	DebugVertex vertex;
	memset(&vertex, 0, sizeof(DebugVertex));

	kmVec3Assign(&vertex.position, positionA);
	kmVec3Assign(&vertex.color, colorA);
	addVertex(&lineVertexBuffer, vertex, size);

	kmVec3Assign(&vertex.position, positionB);
	kmVec3Assign(&vertex.color, colorB);
	addVertex(&lineVertexBuffer, vertex, size);
}

void drawPrimitives(
	DebugVertexBuffer *vertexBuffer,
	GLenum primitiveType,
	const char *name)
{
	if (vertexBuffer->numVertices == 0)
	{
		return;
	}

	glBindVertexArray(vertexBuffer->vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vertexBuffer);

	void *buffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	memcpy(
		buffer,
		vertexBuffer->vertices,
		sizeof(DebugVertex) * MAX_DEBUG_VERTEX_COUNT);
	glUnmapBuffer(GL_ARRAY_BUFFER);

	for (uint8 i = 0; i < NUM_DEBUG_VERTEX_ATTRIBUTES; i++)
	{
		glEnableVertexAttribArray(i);
	}

	glDrawArrays(primitiveType, 0, vertexBuffer->numVertices);
	logGLError(false, "Error when drawing debug %s", name);

	for (uint8 j = 0; j < NUM_DEBUG_VERTEX_ATTRIBUTES; j++)
	{
		glDisableVertexAttribArray(j);
	}

	clearVertices(vertexBuffer);
}