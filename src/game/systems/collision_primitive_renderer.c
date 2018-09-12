#include "defines.h"

#include "core/log.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/model.h"

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

#include <kazmath/mat4.h>
#include <kazmath/mat3.h>
#include <kazmath/quaternion.h>

#include <string.h>
#include <malloc.h>
#include <stdio.h>

#define BOX_MODEL_NAME "box"
#define SPHERE_MODEL_NAME "sphere"
#define CAPSULE_MODEL_NAME "cylinder"

internal GLuint shaderProgram;

internal Uniform modelUniform;
internal Uniform viewUniform;
internal Uniform projectionUniform;

internal Uniform hasAnimationsUniform;

internal Uniform useCustomColorUniform;
internal Uniform customColorUniform;

internal uint32 collisionPrimitiveRendererRefCount = 0;

internal UUID transformComponentID = {};
internal UUID cameraComponentID = {};
internal UUID debugCollisionPrimitiveComponentID = {};
internal UUID boxComponentID = {};
internal UUID sphereComponentID = {};
internal UUID capsuleComponentID = {};

extern real64 alpha;

internal void drawCollisionPrimitives(
	Scene *scene,
	UUID entity,
	TransformComponent *transformComponent,
	DebugCollisionPrimitiveComponent *debugCollisionPrimitive);

internal
void initCollisionPrimitiveRendererSystem(Scene *scene)
{
	if (collisionPrimitiveRendererRefCount == 0)
	{
		LOG("Initializing collision primitive renderer...\n");

		loadModel(BOX_MODEL_NAME);
		loadModel(SPHERE_MODEL_NAME);
		loadModel(CAPSULE_MODEL_NAME);

		createShaderProgram(
			"resources/shaders/model.vert",
			NULL,
			NULL,
			NULL,
			"resources/shaders/model.frag",
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
			"hasAnimations",
			UNIFORM_BOOL,
			&hasAnimationsUniform);

		getUniform(
			shaderProgram,
			"useCustomColor",
			UNIFORM_BOOL,
			&useCustomColorUniform);
		getUniform(
			shaderProgram,
			"customColor",
			UNIFORM_VEC3,
			&customColorUniform);

		LOG("Successfully initialized collision primitive renderer\n");
	}

	collisionPrimitiveRendererRefCount++;
}

internal
void beginCollisionPrimitiveRendererSystem(Scene *scene, real64 dt)
{
	glUseProgram(shaderProgram);

	if (cameraSetUniforms(
		scene,
		viewUniform,
		projectionUniform) == -1)
	{
		return;
	}
}

internal
void runCollisionPrimitiveRendererSystem(Scene *scene, UUID entity, real64 dt)
{
	if (!sceneGetComponentFromEntity(
		scene,
		scene->mainCamera,
		cameraComponentID))
	{
		return;
	}

	DebugCollisionPrimitiveComponent *debugCollisionPrimitive =
		sceneGetComponentFromEntity(
			scene,
			entity,
			debugCollisionPrimitiveComponentID);

	if (!debugCollisionPrimitive->visible)
	{
		return;
	}

	TransformComponent *transform = sceneGetComponentFromEntity(
		scene,
		entity,
		transformComponentID);

	drawCollisionPrimitives(
		scene,
		entity,
		transform,
		debugCollisionPrimitive);
}

internal
void endCollisionPrimitiveRendererSystem(Scene *scene, real64 dt)
{
	glUseProgram(0);
}

internal
void shutdownCollisionPrimitiveRendererSystem(Scene *scene)
{
	if (--collisionPrimitiveRendererRefCount == 0)
	{
		LOG("Shutting down collision primitive renderer...\n");

		freeModel(BOX_MODEL_NAME);
		freeModel(SPHERE_MODEL_NAME);
		freeModel(CAPSULE_MODEL_NAME);

		glDeleteProgram(shaderProgram);

		LOG("Successfully shut down collision primitive renderer\n");
	}
}

System createCollisionPrimitiveRendererSystem(void)
{
	System system = {};

	transformComponentID = idFromName("transform");
	cameraComponentID = idFromName("camera");
	debugCollisionPrimitiveComponentID =
		idFromName("debug_collision_primitive");
	boxComponentID = idFromName("box");
	sphereComponentID = idFromName("sphere");
	capsuleComponentID = idFromName("capsule");

	system.componentTypes = createList(sizeof(UUID));
	listPushFront(&system.componentTypes, &transformComponentID);
	listPushFront(&system.componentTypes, &debugCollisionPrimitiveComponentID);

	system.init = &initCollisionPrimitiveRendererSystem;
	system.begin = &beginCollisionPrimitiveRendererSystem;
	system.run = &runCollisionPrimitiveRendererSystem;
	system.end = &endCollisionPrimitiveRendererSystem;
	system.shutdown = &shutdownCollisionPrimitiveRendererSystem;

	return system;
}

void drawCollisionPrimitives(
	Scene *scene,
	UUID entity,
	TransformComponent *transformComponent,
	DebugCollisionPrimitiveComponent *debugCollisionPrimitive)
{
	BoxComponent *box = sceneGetComponentFromEntity(
		scene,
		entity,
		boxComponentID);
	SphereComponent *sphere = sceneGetComponentFromEntity(
		scene,
		entity,
		sphereComponentID);
	CapsuleComponent *capsule = sceneGetComponentFromEntity(
		scene,
		entity,
		capsuleComponentID);

	bool skip = false;

	const char *modelName = "";
	kmVec3 color;

	if (box)
	{
		modelName = BOX_MODEL_NAME;
		kmVec3Assign(&color, &debugCollisionPrimitive->boxColor);
	}
	else if (sphere)
	{
		modelName = SPHERE_MODEL_NAME;
		kmVec3Assign(&color, &debugCollisionPrimitive->sphereColor);
	}
	else if (capsule)
	{
		modelName = CAPSULE_MODEL_NAME;
		kmVec3Assign(&color, &debugCollisionPrimitive->capsuleColor);
	}
	else
	{
		skip = true;
	}

	if (!skip)
	{
		Model *model = getModel(modelName);
		if (!model)
		{
			skip = true;
		}

		if (!skip)
		{
			TransformComponent transform = *transformComponent;

			if (box)
			{
				kmVec3Assign(&transform.lastGlobalScale, &box->bounds);
				kmVec3Scale(
					&transform.lastGlobalScale,
					&transform.lastGlobalScale,
					2.0f);

				kmVec3Assign(&transform.globalScale, &box->bounds);
				kmVec3Scale(
					&transform.globalScale,
					&transform.globalScale,
					2.0f);
			}
			else if (sphere)
			{
				kmVec3Fill(
					&transform.lastGlobalScale,
					sphere->radius,
					sphere->radius,
					sphere->radius);
				kmVec3Scale(
					&transform.lastGlobalScale,
					&transform.lastGlobalScale,
					2.0f);

				kmVec3Fill(
					&transform.globalScale,
					sphere->radius,
					sphere->radius,
					sphere->radius);
				kmVec3Scale(
					&transform.globalScale,
					&transform.globalScale,
					2.0f);
			}
			else if (capsule)
			{
				kmVec3 bounds;
				bounds.x = capsule->radius * 2;
				bounds.y = capsule->radius * 2;
				bounds.z = capsule->length + 2 * capsule->radius;

				kmVec3Assign(&transform.lastGlobalScale, &bounds);
				kmVec3Assign(&transform.globalScale, &bounds);
			}

			kmMat4 worldMatrix = tGetInterpolatedTransformMatrix(
				&transform,
				alpha);

			setUniform(modelUniform, 1, &worldMatrix);

			bool hasAnimations = false;
			setUniform(hasAnimationsUniform, 1, &hasAnimations);

			for (uint32 i = 0; i < model->numSubsets; i++)
			{
				Subset *subset = &model->subsets[i];
				Mesh *mesh = &subset->mesh;

				glBindVertexArray(mesh->vertexArray);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);

				for (uint8 j = 0; j < NUM_VERTEX_ATTRIBUTES; j++)
				{
					glEnableVertexAttribArray(j);
				}

				bool useCustomColor = true;
				setUniform(useCustomColorUniform, 1, &useCustomColor);
				setUniform(customColorUniform, 1, &color);

				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

				real32 lineWidth;
				glGetFloatv(GL_LINE_WIDTH, &lineWidth);
				glLineWidth(debugCollisionPrimitive->lineWidth);

				glDisable(GL_CULL_FACE);

				glDrawElements(
					GL_TRIANGLES,
					mesh->numIndices,
					GL_UNSIGNED_INT,
					NULL);

				logGLError(
					false,
					"Error when drawing collision primitive (%s), entity (%s)",
					model->name.string,
					entity.string);

				glLineWidth(lineWidth);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

				glEnable(GL_CULL_FACE);

				for (uint8 j = 0; j < NUM_VERTEX_ATTRIBUTES; j++)
				{
					glDisableVertexAttribArray(j);
				}

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
			}
		}
	}

	if (debugCollisionPrimitive->recursive)
	{
		UUID child = transformComponent->firstChild;

		do {
			TransformComponent *childTransform = sceneGetComponentFromEntity(
				scene,
				child,
				transformComponentID);

			if (childTransform)
			{
				drawCollisionPrimitives(
					scene,
					child,
					childTransform,
					debugCollisionPrimitive);
				child = childTransform->nextSibling;
			}
			else
			{
				break;
			}
		} while (true);
	}
}