#include "defines.h"

#include "renderer/renderer_types.h"
#include "renderer/renderer_utilities.h"
#include "renderer/shader.h"

#include "ECS/ecs_types.h"

#include "components/component_types.h"
#include "components/camera.h"

#include "data/data_types.h"
#include "data/list.h"

#include <kazmath/vec2.h>

#include <stddef.h>

internal uint32 lineRendererRefCount = 0;

#define MAX_LINE_COUNT 2048
#define MAX_LINE_VERTEX_COUNT MAX_LINE_COUNT * 2

#define NUM_LINE_VERTEX_ATTRIBUTES 2

typedef struct line_vertex_t {
	kmVec3 position;
	kmVec3 color;
} LineVertex;

internal LineVertex vertices[MAX_LINE_VERTEX_COUNT];
internal uint32 numVertices;

internal GLuint vertexBuffer;
internal GLuint vertexArray;

internal GLuint shaderProgram;

internal Uniform viewUniform;
internal Uniform projectionUniform;

void drawLine(
	const kmVec3 *positionA,
	const kmVec3 *positionB,
	const kmVec3 *color);

internal void clearVertices(void);
internal void addVertex(LineVertex vertex);

internal void initLineRendererSystem(Scene *scene)
{
	if (lineRendererRefCount == 0)
	{
		clearVertices();

		createShaderProgram(
			"resources/shaders/line.vert",
			NULL,
			NULL,
			NULL,
			"resources/shaders/line.frag",
			NULL,
			&shaderProgram);

		glGenBuffers(1, &vertexBuffer);
		glGenVertexArrays(1, &vertexArray);

		uint32 bufferIndex = 0;

		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(
			GL_ARRAY_BUFFER,
			sizeof(LineVertex) * MAX_LINE_VERTEX_COUNT,
			NULL,
			GL_DYNAMIC_DRAW);

		glBindVertexArray(vertexArray);
		glVertexAttribPointer(
			bufferIndex++,
			3,
			GL_FLOAT,
			GL_FALSE,
			sizeof(LineVertex),
			(GLvoid*)offsetof(LineVertex, position));

		glVertexAttribPointer(
			bufferIndex++,
			3,
			GL_FLOAT,
			GL_FALSE,
			sizeof(LineVertex),
			(GLvoid*)offsetof(LineVertex, color));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		getUniform(shaderProgram, "view", UNIFORM_MAT4, &viewUniform);
		getUniform(
			shaderProgram,
			"projection",
			UNIFORM_MAT4,
			&projectionUniform);
	}

	lineRendererRefCount++;
}

internal void beginLineRendererSystem(Scene *scene, real64 dt)
{
	glUseProgram(shaderProgram);

	if (cameraSetUniforms(
		scene,
		viewUniform,
		projectionUniform) == -1)
	{
		return;
	}

	glBindVertexArray(vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	void *buffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	memcpy(
		buffer,
		vertices,
		sizeof(LineVertex) * MAX_LINE_VERTEX_COUNT);
	glUnmapBuffer(GL_ARRAY_BUFFER);

	for (uint8 i = 0; i < NUM_LINE_VERTEX_ATTRIBUTES; i++)
	{
		glEnableVertexAttribArray(i);
	}

	glDrawArrays(GL_LINES, 0, numVertices);
	logGLError(false, "Error when drawing line");
}

internal void endLineRendererSystem(Scene *scene, real64 dt)
{
	for (uint8 j = 0; j < NUM_LINE_VERTEX_ATTRIBUTES; j++)
	{
		glDisableVertexAttribArray(j);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(0);

	clearVertices();
}

internal void shutdownLineRendererSystem(Scene *scene)
{
	if (--lineRendererRefCount == 0)
	{
		glBindVertexArray(vertexArray);
		glDeleteBuffers(1, &vertexBuffer);
		glBindVertexArray(0);

		glDeleteVertexArrays(1, &vertexArray);

		glDeleteProgram(shaderProgram);
	}
}

System createLineRendererSystem(void)
{
	System system = {};
	system.componentTypes = createList(sizeof(UUID));

	system.init = &initLineRendererSystem;
	system.begin = &beginLineRendererSystem;
	system.end = &endLineRendererSystem;
	system.shutdown = &shutdownLineRendererSystem;

	return system;
}

void drawLine(
	const kmVec3 *positionA,
	const kmVec3 *positionB,
	const kmVec3 *color)
{
	LineVertex vertex;
	memset(&vertex, 0, sizeof(LineVertex));

	kmVec3Assign(&vertex.position, positionA);
	kmVec3Assign(&vertex.color, color);
	addVertex(vertex);

	kmVec3Assign(&vertex.position, positionB);
	addVertex(vertex);
}

void clearVertices(void)
{
	memset(vertices, 0, sizeof(LineVertex) * MAX_LINE_VERTEX_COUNT);
	numVertices = 0;
}

void addVertex(LineVertex vertex)
{
	vertices[numVertices++] = vertex;
}