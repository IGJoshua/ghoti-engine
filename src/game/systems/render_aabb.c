#include "defines.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/list.h"
#include "data/hash_map.h"

#include "asset_management/asset_manager_types.h"

#include "renderer/renderer_types.h"
#include "renderer/shader.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "components/component_types.h"

#include <kazmath/mat3.h>
#include <kazmath/mat4.h>

#include <stdio.h>
#include <malloc.h>
#include <stddef.h>
#include <string.h>

// internal Shader vertShader;
// internal Shader fragShader;
// internal ShaderPipeline pipeline;
// internal Uniform modelUniform;
// internal Uniform viewUniform;
// internal Uniform projectionUniform;

// internal bool rendererActive;

// internal HashMap sceneLineData;

internal UUID transformComponentID = {};
internal UUID boxComponentID = {};
internal UUID cameraComponentID = {};
internal UUID collisionComponentID = {};
internal UUID collisionTreeNodeComponentID = {};

// #define MAX_LINES 4096

// typedef struct scene_line_allocator_t
// {
// 	uint32 numVerts;
// 	uint32 maxVerts;
// 	GLuint vertexBuffer;
// 	GLuint vertexArray;
// 	Vertex verts[MAX_LINES * 2];
// } SceneLineAllocator;

void initRenderBoxSystem(Scene *scene)
{
	// if (!rendererActive)
	// {
	// 	// Initialize all the shaders
	// 	if (compileShaderFromFile(
	// 			"resources/shaders/base.vert",
	// 			SHADER_VERTEX,
	// 			&vertShader) == -1)
	// 	{
	// 		LOG("Unable to compile vertex shader for box renderer\n");
	// 	}

	// 	if (compileShaderFromFile(
	// 			"resources/shaders/flat_color.frag",
	// 			SHADER_FRAGMENT,
	// 			&fragShader) == -1)
	// 	{
	// 		LOG("Unable to compile fragment shader for box renderer\n");
	// 	}

	// 	Shader *program[2];
	// 	program[0] = &vertShader;
	// 	program[1] = &fragShader;

	// 	if (composeShaderPipeline(program, 2, &pipeline) == -1)
	// 	{
	// 		LOG("Unable to compose box shader pipeline\n");
	// 	}

	// 	freeShader(vertShader);
	// 	freeShader(fragShader);
	// 	free(pipeline.shaders);
	// 	pipeline.shaderCount = 0;

	// 	if (getUniform(pipeline, "model", UNIFORM_MAT4, &modelUniform) == -1)
	// 	{
	// 		LOG("Unable to get model component uniform\n");
	// 	}

	// 	if (getUniform(pipeline, "view", UNIFORM_MAT4, &viewUniform) == -1)
	// 	{
	// 		LOG("Unable to get view component uniform\n");
	// 	}

	// 	if (getUniform(
	// 			pipeline,
	// 			"projection",
	// 			UNIFORM_MAT4,
	// 			&projectionUniform) == -1)
	// 	{
	// 		LOG("Unable to get projection component uniform\n");
	// 	}

	// 	rendererActive = true;
	// }

	// // Initialize the line renderer
	// if(hashMapGetData(&sceneLineData, &scene))
	// {
	// 	hashMapDelete(&sceneLineData, &scene);
	// }
	// SceneLineAllocator sceneLineAllocator = {};

	// sceneLineAllocator.maxVerts = MAX_LINES * 2;
	// sceneLineAllocator.numVerts = 0;
	// for (uint32 i = 0; i < MAX_LINES * 2; ++i)
	// {
	// 	kmVec3Fill(&sceneLineAllocator.verts[i].position, 0, 0, 0);
	// 	kmVec3Fill(&sceneLineAllocator.verts[i].normal, 0, 0, 0);
	// 	kmVec3Fill(&sceneLineAllocator.verts[i].tangent, 0, 0, 0);
	// 	kmVec3Fill(&sceneLineAllocator.verts[i].bitangent, 0, 0, 0);
	// 	kmVec4Fill(&sceneLineAllocator.verts[i].color, 0, 0, 0, 1);
	// 	for (uint32 j = 0; j < MATERIAL_COMPONENT_TYPE_COUNT; ++j)
	// 	{
	// 		kmVec2Fill(&sceneLineAllocator.verts[i].uv[j], 0, 0);
	// 	}
	// }

	// glGenBuffers(1, &sceneLineAllocator.vertexBuffer);
	// glGenVertexArrays(1, &sceneLineAllocator.vertexArray);

	// uint32 bufferIndex = 0;

	// glBindBuffer(GL_ARRAY_BUFFER, sceneLineAllocator.vertexBuffer);
	// glBufferData(
	// 	GL_ARRAY_BUFFER,
	// 	sizeof(Vertex) * MAX_LINES * 2,
	// 	&sceneLineAllocator.verts,
	// 	GL_DYNAMIC_DRAW);

	// glBindVertexArray(sceneLineAllocator.vertexArray);
	// glVertexAttribPointer(
	// 	bufferIndex++,
	// 	3,
	// 	GL_FLOAT,
	// 	GL_FALSE,
	// 	sizeof(Vertex),
	// 	(GLvoid*)offsetof(Vertex, position));
	// glVertexAttribPointer(
	// 	bufferIndex++,
	// 	4,
	// 	GL_FLOAT,
	// 	GL_FALSE,
	// 	sizeof(Vertex),
	// 	(GLvoid*)offsetof(Vertex, color));
	// glVertexAttribPointer(
	// 	bufferIndex++,
	// 	3,
	// 	GL_FLOAT,
	// 	GL_FALSE,
	// 	sizeof(Vertex),
	// 	(GLvoid*)offsetof(Vertex, normal));
	// glVertexAttribPointer(
	// 	bufferIndex++,
	// 	3,
	// 	GL_FLOAT,
	// 	GL_FALSE,
	// 	sizeof(Vertex),
	// 	(GLvoid*)offsetof(Vertex, tangent));
	// glVertexAttribPointer(
	// 	bufferIndex++,
	// 	3,
	// 	GL_FLOAT,
	// 	GL_FALSE,
	// 	sizeof(Vertex),
	// 	(GLvoid*)offsetof(Vertex, bitangent));

	// for (uint32 j = 0; j < MATERIAL_COMPONENT_TYPE_COUNT; j++)
	// {
	// 	glVertexAttribPointer(
	// 		bufferIndex++,
	// 		2,
	// 		GL_FLOAT,
	// 		GL_FALSE,
	// 		sizeof(Vertex),
	// 		(GLvoid*)offsetof(Vertex, uv[j]));
	// }

	// glBindBuffer(GL_ARRAY_BUFFER, 0);
	// glBindVertexArray(0);

	// hashMapInsert(&sceneLineData, &scene, &sceneLineAllocator);
}

// internal kmVec4 noCollision = {0, 1, 0, 1};
// internal kmVec4 collision = {1, 0, 0, 1};

internal
void beginRenderBoxSystem(Scene *scene, real64 dt)
{
	// SceneLineAllocator *sceneLineAllocator = hashMapGetData(
	// 	&sceneLineData,
	// 	&scene);

	// sceneLineAllocator->numVerts = 0;
}

// internal inline
// void addLine(
// 	SceneLineAllocator *sceneLineAllocator,
// 	kmVec3 pos1,
// 	kmVec3 pos2,
// 	bool collided)
// {
// 	sceneLineAllocator->verts[sceneLineAllocator->numVerts].position = pos1;
// 	sceneLineAllocator->verts[sceneLineAllocator->numVerts++].color = collided ? collision : noCollision;
// 	sceneLineAllocator->verts[sceneLineAllocator->numVerts].position = pos2;
// 	sceneLineAllocator->verts[sceneLineAllocator->numVerts++].color = collided ? collision : noCollision;
// }

void runRenderBoxSystem(Scene *scene, UUID entityID, real64 dt)
{
	// SceneLineAllocator *sceneLineAllocator = hashMapGetData(
	// 	&sceneLineData,
	// 	&scene);
	// ASSERT(sceneLineAllocator);

	// TransformComponent *transform = sceneGetComponentFromEntity(
	// 	scene,
	// 	entityID,
	// 	transformComponentID);

	// BoxComponent *box = sceneGetComponentFromEntity(
	// 	scene,
	// 	entityID,
	// 	boxComponentID);

	// CollisionTreeNode *treeNode = sceneGetComponentFromEntity(
	// 	scene,
	// 	entityID,
	// 	collisionTreeNodeComponentID);

	// CollisionComponent *collision = sceneGetComponentFromEntity(
	// 	scene,
	// 	treeNode->collisionVolume,
	// 	collisionComponentID);

	// bool collided = strcmp(collision->hitList.string, "");

	// kmVec3 pos1;
	// kmVec3 pos2;
	// kmVec3 bounds;
	// kmVec3Assign(&bounds, &box->bounds);
	// kmVec3Mul(&bounds, &bounds, &transform->globalScale);

	// // Top front left
	// kmVec3Fill(
	// 	&pos1,
	// 	transform->globalPosition.x - bounds.x,
	// 	transform->globalPosition.y + bounds.y,
	// 	transform->globalPosition.z + bounds.z);
	// kmVec3Fill(
	// 	&pos2,
	// 	transform->globalPosition.x + bounds.x,
	// 	transform->globalPosition.y + bounds.y,
	// 	transform->globalPosition.z + bounds.z);
	// addLine(sceneLineAllocator, pos1, pos2, collided);

	// kmVec3Fill(
	// 	&pos2,
	// 	transform->globalPosition.x - bounds.x,
	// 	transform->globalPosition.y - bounds.y,
	// 	transform->globalPosition.z + bounds.z);
	// addLine(sceneLineAllocator, pos1, pos2, collided);

	// kmVec3Fill(
	// 	&pos2,
	// 	transform->globalPosition.x - bounds.x,
	// 	transform->globalPosition.y + bounds.y,
	// 	transform->globalPosition.z - bounds.z);
	// addLine(sceneLineAllocator, pos1, pos2, collided);

	// // Top front right
	// kmVec3Fill(
	// 	&pos1,
	// 	transform->globalPosition.x + bounds.x,
	// 	transform->globalPosition.y + bounds.y,
	// 	transform->globalPosition.z + bounds.z);
	// kmVec3Fill(
	// 	&pos2,
	// 	transform->globalPosition.x + bounds.x,
	// 	transform->globalPosition.y + bounds.y,
	// 	transform->globalPosition.z - bounds.z);
	// addLine(sceneLineAllocator, pos1, pos2, collided);

	// kmVec3Fill(
	// 	&pos2,
	// 	transform->globalPosition.x + bounds.x,
	// 	transform->globalPosition.y - bounds.y,
	// 	transform->globalPosition.z + bounds.z);
	// addLine(sceneLineAllocator, pos1, pos2, collided);

	// // Bottom front left
	// kmVec3Fill(
	// 	&pos1,
	// 	transform->globalPosition.x - bounds.x,
	// 	transform->globalPosition.y - bounds.y,
	// 	transform->globalPosition.z + bounds.z);
	// kmVec3Fill(
	// 	&pos2,
	// 	transform->globalPosition.x - bounds.x,
	// 	transform->globalPosition.y - bounds.y,
	// 	transform->globalPosition.z - bounds.z);
	// addLine(sceneLineAllocator, pos1, pos2, collided);

	// kmVec3Fill(
	// 	&pos2,
	// 	transform->globalPosition.x + bounds.x,
	// 	transform->globalPosition.y - bounds.y,
	// 	transform->globalPosition.z + bounds.z);
	// addLine(sceneLineAllocator, pos1, pos2, collided);

	// // Bottom front right
	// kmVec3Fill(
	// 	&pos1,
	// 	transform->globalPosition.x + bounds.x,
	// 	transform->globalPosition.y - bounds.y,
	// 	transform->globalPosition.z + bounds.z);
	// kmVec3Fill(
	// 	&pos2,
	// 	transform->globalPosition.x + bounds.x,
	// 	transform->globalPosition.y - bounds.y,
	// 	transform->globalPosition.z - bounds.z);
	// addLine(sceneLineAllocator, pos1, pos2, collided);

	// // Back top left
	// kmVec3Fill(
	// 	&pos1,
	// 	transform->globalPosition.x - bounds.x,
	// 	transform->globalPosition.y + bounds.y,
	// 	transform->globalPosition.z - bounds.z);
	// kmVec3Fill(
	// 	&pos2,
	// 	transform->globalPosition.x + bounds.x,
	// 	transform->globalPosition.y + bounds.y,
	// 	transform->globalPosition.z - bounds.z);
	// addLine(sceneLineAllocator, pos1, pos2, collided);

	// kmVec3Fill(
	// 	&pos2,
	// 	transform->globalPosition.x - bounds.x,
	// 	transform->globalPosition.y - bounds.y,
	// 	transform->globalPosition.z - bounds.z);
	// addLine(sceneLineAllocator, pos1, pos2, collided);

	// // Back bottom right
	// kmVec3Fill(
	// 	&pos1,
	// 	transform->globalPosition.x + bounds.x,
	// 	transform->globalPosition.y - bounds.y,
	// 	transform->globalPosition.z - bounds.z);
	// kmVec3Fill(
	// 	&pos2,
	// 	transform->globalPosition.x - bounds.x,
	// 	transform->globalPosition.y - bounds.y,
	// 	transform->globalPosition.z - bounds.z);
	// addLine(sceneLineAllocator, pos1, pos2, collided);

	// kmVec3Fill(
	// 	&pos2,
	// 	transform->globalPosition.x + bounds.x,
	// 	transform->globalPosition.y + bounds.y,
	// 	transform->globalPosition.z - bounds.z);
	// addLine(sceneLineAllocator, pos1, pos2, collided);
}

// internal CameraComponent *camera = 0;
// internal TransformComponent *cameraTransform = 0;
// internal kmMat4 view = {};
// internal kmMat4 projection = {};

// extern real64 alpha;

void endRenderBoxSystem(Scene *scene, real64 dt)
{
	// SceneLineAllocator *sceneLineAllocator = hashMapGetData(
	// 	&sceneLineData,
	// 	&scene);
	// ASSERT(sceneLineAllocator);

	// camera = sceneGetComponentFromEntity(
	// 	scene,
	// 	scene->mainCamera,
	// 	cameraComponentID);
	// cameraTransform = sceneGetComponentFromEntity(
	// 	scene,
	// 	scene->mainCamera,
	// 	transformComponentID);

	// kmQuaternion cameraRotationQuat;
	// kmQuaternionSlerp(
	// 	&cameraRotationQuat,
	// 	&cameraTransform->lastGlobalRotation,
	// 	&cameraTransform->globalRotation,
	// 	alpha);

	// kmMat3 cameraRotation;
	// kmMat3FromRotationQuaternion(&cameraRotation, &cameraRotationQuat);

	// kmVec3 cameraPosition;
	// kmVec3Lerp(
	// 	&cameraPosition,
	// 	&cameraTransform->lastGlobalPosition,
	// 	&cameraTransform->globalPosition,
	// 	alpha);

	// kmMat4RotationTranslation(
	// 	&view,
	// 	&cameraRotation,
	// 	&cameraPosition);
	// kmMat4Inverse(&view, &view);

	// kmMat4PerspectiveProjection(
	// 	&projection,
	// 	camera->fov,
	// 	camera->aspectRatio,
	// 	camera->nearPlane,
	// 	camera->farPlane);

	// bindShaderPipeline(pipeline);

	// if (setUniform(viewUniform, &view) == -1)
	// {
	// 	LOG("Unable to set view uniform\n");
	// }

	// if (setUniform(projectionUniform, &projection) == -1)
	// {
	// 	LOG("Unable to set projection uniform\n");
	// }

	// kmMat4 model;
	// kmMat4Identity(&model);

	// if (setUniform(modelUniform, &model) == -1)
	// {
	// 	LOG("Unable to set the model uniform\n");
	// }

	// glBindVertexArray(sceneLineAllocator->vertexArray);
	// glBindBuffer(GL_ARRAY_BUFFER, sceneLineAllocator->vertexBuffer);

	// void *buffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	// ASSERT(buffer);
	// memcpy(buffer, sceneLineAllocator->verts, sizeof(Vertex) * MAX_LINES * 2);
	// glUnmapBuffer(GL_ARRAY_BUFFER);

	// for (uint8 j = 0; j < NUM_VERTEX_ATTRIBUTES; ++j)
	// {
	// 	glEnableVertexAttribArray(j);
	// }

	// glDrawArrays(GL_LINES, 0, sceneLineAllocator->numVerts);

	// for (uint8 j = 0; j < NUM_VERTEX_ATTRIBUTES; ++j)
	// {
	// 	glDisableVertexAttribArray(j);
	// }
}

void shutdownRenderBoxSystem(Scene *scene)
{
	// SceneLineAllocator *sceneLineAllocator =
	// 	hashMapGetData(&sceneLineData, &scene);
	// ASSERT(sceneLineAllocator);
	// glBindVertexArray(sceneLineAllocator->vertexArray);
	// glDeleteBuffers(1, &sceneLineAllocator->vertexBuffer);
	// glBindVertexArray(0);

	// glDeleteVertexArrays(1, &sceneLineAllocator->vertexArray);

	// hashMapDelete(&sceneLineData, &scene);
}

// internal
// int32 ptrEq(void *thing1, void *thing2)
// {
// 	return *(uint64*)thing1 != *(uint64*)thing2;
// }

System createRenderBoxSystem(void)
{
	transformComponentID = idFromName("transform");
	boxComponentID = idFromName("box");
	collisionComponentID = idFromName("collision");
	cameraComponentID = idFromName("camera");
	collisionTreeNodeComponentID = idFromName("collision_tree_node");

	// sceneLineData = createHashMap(
	// 	sizeof(Scene *), sizeof(SceneLineAllocator), 7, &ptrEq);

	System ret = {};

	ret.componentTypes = createList(sizeof(UUID));
	listPushFront(&ret.componentTypes, &transformComponentID);
	listPushFront(&ret.componentTypes, &boxComponentID);
	listPushFront(&ret.componentTypes, &collisionTreeNodeComponentID);

	ret.init = &initRenderBoxSystem;
	ret.begin = &beginRenderBoxSystem;
	ret.run = &runRenderBoxSystem;
	ret.end = &endRenderBoxSystem;
	ret.shutdown = &shutdownRenderBoxSystem;

	return ret;
}
