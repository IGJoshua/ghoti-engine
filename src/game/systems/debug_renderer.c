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
#include "data/hash_map.h"
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

#define VERTEX_DATA_BUCKET_COUNT 127

typedef struct debug_vertex_data_t
{
	DebugVertex vertices[MAX_DEBUG_VERTEX_COUNT];
	uint32 numVertices;
} DebugVertexData;

typedef struct debug_vertex_buffer_t
{
	HashMap vertexData;
	uint32 numVertices;
	GLuint vertexBuffer;
	GLuint vertexArray;
} DebugVertexBuffer;

DebugVertexBuffer pointVertexBuffer;
DebugVertexBuffer lineVertexBuffer;

#define VERTEX_SHADER_FILE "resources/shaders/debug.vert"
#define FRAGMENT_SHADER_FILE "resources/shaders/debug.frag"

internal GLuint shaderProgram;

internal Uniform viewUniform;
internal Uniform projectionUniform;

internal UUID transformComponentID = {};
internal UUID cameraComponentID = {};
internal UUID debugPrimitiveComponentID = {};
internal UUID debugPointComponentID = {};
internal UUID debugLineComponentID = {};
internal UUID debugTransformComponentID = {};

internal CameraComponent *camera;
internal TransformComponent *cameraTransform;

extern real64 alpha;

internal void initializeVertexBuffer(DebugVertexBuffer *vertexBuffer);
internal void clearVertexBuffer(DebugVertexBuffer *vertexBuffer);
internal void clearVertices(DebugVertexData *vertexData);

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
internal void addTransforms(
	Scene *scene,
	TransformComponent *transform,
	DebugTransformComponent *debugTransform);
internal void addTransform(
	const kmVec3 *position,
	const kmQuaternion *rotation,
	const kmVec3 *scale,
	const kmVec3 *xAxisColor,
	const kmVec3 *xAxisEndpointColor,
	const kmVec3 *yAxisColor,
	const kmVec3 *yAxisEndpointColor,
	const kmVec3 *zAxisColor,
	const kmVec3 *zAxisEndpointColor,
	real32 lineWidth,
	real32 sizeScale);

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
			VERTEX_SHADER_FILE,
			NULL,
			NULL,
			NULL,
			FRAGMENT_SHADER_FILE,
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
	DebugPointComponent *debugPoint = sceneGetComponentFromEntity(
		scene,
		entity,
		debugPointComponentID);
	DebugLineComponent *debugLine = sceneGetComponentFromEntity(
		scene,
		entity,
		debugLineComponentID);
	DebugTransformComponent *debugTransform = sceneGetComponentFromEntity(
		scene,
		entity,
		debugTransformComponentID);

	if (!debugPrimitive->visible)
	{
		return;
	}

	kmVec3 position;
	tGetInterpolatedTransform(transform, &position, NULL, NULL, alpha);

	if (debugPoint)
	{
		addPoint(&position, &debugPoint->color, debugPoint->size);
	}
	else if (debugLine)
	{
		TransformComponent *endpointTransform =  sceneGetComponentFromEntity(
			scene,
			debugLine->endpoint,
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
				&debugLine->color,
				&debugLine->endpointColor,
				debugLine->lineWidth);
		}
	}
	else if (debugTransform)
	{
		addTransforms(scene, transform, debugTransform);
	}
}

internal void endDebugRendererSystem(Scene *scene, real64 dt)
{
	camera = sceneGetComponentFromEntity(
		scene,
		scene->mainCamera,
		cameraComponentID);

	cameraTransform = sceneGetComponentFromEntity(
		scene,
		scene->mainCamera,
		transformComponentID);

	if (!camera || !cameraTransform)
	{
		return;
	}

	glUseProgram(shaderProgram);

	cameraSetUniforms(camera, cameraTransform, viewUniform, projectionUniform);

	drawPrimitives(&pointVertexBuffer, GL_POINTS, "points");
	drawPrimitives(&lineVertexBuffer, GL_LINES, "lines");

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

		freeHashMap(&pointVertexBuffer.vertexData);
		freeHashMap(&lineVertexBuffer.vertexData);

		LOG("Successfully shut down debug renderer\n");
	}
}

System createDebugRendererSystem(void)
{
	System system = {};

	transformComponentID = idFromName("transform");
	cameraComponentID = idFromName("camera");
	debugPrimitiveComponentID = idFromName("debug_primitive");
	debugPointComponentID = idFromName("debug_point");
	debugLineComponentID = idFromName("debug_line");
	debugTransformComponentID = idFromName("debug_transform");

	system.componentTypes = createList(sizeof(UUID));
	listPushFront(&system.componentTypes, &transformComponentID);
	listPushFront(&system.componentTypes, &debugPrimitiveComponentID);

	system.init = &initDebugRendererSystem;
	system.run = &runDebugRendererSystem;
	system.end = &endDebugRendererSystem;
	system.shutdown = &shutdownDebugRendererSystem;

	return system;
}

internal int32 floatcmp(void *a, void *b)
{
	return *(real32*)a != *(real32*)b;
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

	vertexBuffer->vertexData = createHashMap(
		sizeof(real32),
		sizeof(DebugVertexData),
		VERTEX_DATA_BUCKET_COUNT,
		(ComparisonOp)&floatcmp);

	clearVertexBuffer(vertexBuffer);
}

void clearVertexBuffer(DebugVertexBuffer *vertexBuffer)
{
	for (HashMapIterator itr = hashMapGetIterator(vertexBuffer->vertexData);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		clearVertices((DebugVertexData*)hashMapIteratorGetValue(itr));
	}

	vertexBuffer->numVertices = 0;
}

void clearVertices(DebugVertexData *vertexData)
{
	memset(
		vertexData->vertices,
		0,
		sizeof(DebugVertex) * MAX_DEBUG_VERTEX_COUNT);
	vertexData->numVertices = 0;
}

void addVertex(DebugVertexBuffer *vertexBuffer, DebugVertex vertex, real32 size)
{
	if (vertexBuffer->numVertices < MAX_DEBUG_VERTEX_COUNT)
	{
		DebugVertexData *vertexData = hashMapGetData(
			vertexBuffer->vertexData,
			&size);

		if (!vertexData)
		{
			DebugVertexData newVertexData;
			clearVertices(&newVertexData);
			hashMapInsert(vertexBuffer->vertexData, &size, &newVertexData);
			vertexData = hashMapGetData(vertexBuffer->vertexData, &size);
		}

		vertexData->vertices[vertexData->numVertices++] = vertex;
		vertexBuffer->numVertices++;
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

void addTransforms(
	Scene *scene,
	TransformComponent *transform,
	DebugTransformComponent *debugTransform)
{
	kmVec3 position;
	kmQuaternion rotation;
	kmVec3 scale;

	tGetInterpolatedTransform(transform, &position, &rotation, &scale, alpha);

	addTransform(
		&position,
		&rotation,
		&scale,
		&debugTransform->xAxisColor,
		&debugTransform->xAxisEndpointColor,
		&debugTransform->yAxisColor,
		&debugTransform->yAxisEndpointColor,
		&debugTransform->zAxisColor,
		&debugTransform->zAxisEndpointColor,
		debugTransform->lineWidth,
		debugTransform->scale);

	if (debugTransform->recursive)
	{
		UUID child = transform->firstChild;

		do {
			TransformComponent *childTransform = sceneGetComponentFromEntity(
				scene,
				child,
				transformComponentID);

			if (childTransform)
			{
				addTransforms(scene, childTransform, debugTransform);
				child = childTransform->nextSibling;
			}
			else
			{
				break;
			}
		} while (true);
	}
}

void addTransform(
	const kmVec3 *position,
	const kmQuaternion *rotation,
	const kmVec3 *scale,
	const kmVec3 *xAxisColor,
	const kmVec3 *xAxisEndpointColor,
	const kmVec3 *yAxisColor,
	const kmVec3 *yAxisEndpointColor,
	const kmVec3 *zAxisColor,
	const kmVec3 *zAxisEndpointColor,
	real32 lineWidth,
	real32 sizeScale)
{
	kmVec3 xAxis;
	kmQuaternionGetRightVec3(&xAxis, rotation);

	kmVec3 yAxis;
	kmQuaternionGetUpVec3(&yAxis, rotation);

	kmVec3 zAxis;
	kmQuaternionGetForwardVec3LH(&zAxis, rotation);

	kmVec3Mul(&xAxis, &xAxis, scale);
	kmVec3Mul(&yAxis, &yAxis, scale);
	kmVec3Mul(&zAxis, &zAxis, scale);

	kmVec3Scale(&xAxis, &xAxis, sizeScale);
	kmVec3Scale(&yAxis, &yAxis, sizeScale);
	kmVec3Scale(&zAxis, &zAxis, sizeScale);

	kmVec3Add(&xAxis, &xAxis, position);
	kmVec3Add(&yAxis, &yAxis, position);
	kmVec3Add(&zAxis, &zAxis, position);

	addLine(position, &xAxis, xAxisColor, xAxisEndpointColor, lineWidth);
	addLine(position, &yAxis, yAxisColor, yAxisEndpointColor, lineWidth);
	addLine(position, &zAxis, zAxisColor, zAxisEndpointColor, lineWidth);
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

	uint32 index = 0;
	for (HashMapIterator itr = hashMapGetIterator(vertexBuffer->vertexData);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		DebugVertexData *vertexData = hashMapIteratorGetValue(itr);
		uint32 size = sizeof(DebugVertex) * vertexData->numVertices;
		memcpy(buffer + index, vertexData->vertices, size);
		index += size;
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);

	for (uint8 i = 0; i < NUM_DEBUG_VERTEX_ATTRIBUTES; i++)
	{
		glEnableVertexAttribArray(i);
	}

	index = 0;
	for (HashMapIterator itr = hashMapGetIterator(vertexBuffer->vertexData);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		real32 primitiveSize = *(real32*)hashMapIteratorGetKey(itr);

		real32 pointSize;
		glGetFloatv(GL_POINT_SIZE, &pointSize);
		glPointSize(primitiveSize);

		real32 lineWidth;
		glGetFloatv(GL_LINE_WIDTH, &lineWidth);
		glLineWidth(primitiveSize);

		DebugVertexData *vertexData = hashMapIteratorGetValue(itr);
		glDrawArrays(primitiveType, index, vertexData->numVertices);
		logGLError(false, "Failed to draw debug %s", name);

		glPointSize(pointSize);
		glLineWidth(primitiveSize);

		index += vertexData->numVertices;
	}

	for (uint8 j = 0; j < NUM_DEBUG_VERTEX_ATTRIBUTES; j++)
	{
		glDisableVertexAttribArray(j);
	}

	clearVertexBuffer(vertexBuffer);
}