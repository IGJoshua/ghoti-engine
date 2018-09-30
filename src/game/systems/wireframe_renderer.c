#include "defines.h"

#include "core/log.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/model.h"
#include "asset_management/animation.h"
#include "asset_management/texture.h"

#include "renderer/renderer_types.h"
#include "renderer/renderer_utilities.h"
#include "renderer/shader.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "components/component_types.h"
#include "components/transform.h"
#include "components/camera.h"
#include "components/animation.h"
#include "components/animator.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include <kazmath/mat4.h>
#include <kazmath/mat3.h>
#include <kazmath/quaternion.h>

#include <string.h>
#include <malloc.h>
#include <stdio.h>

#define VERTEX_SHADER_FILE "resources/shaders/model.vert"
#define FRAGMENT_SHADER_FILE "resources/shaders/model.frag"

internal GLuint shaderProgram;

internal Uniform modelUniform;
internal Uniform viewUniform;
internal Uniform projectionUniform;

internal Uniform hasAnimationsUniform;
internal Uniform boneTransformsUniform;

internal Uniform useCustomColorUniform;
internal Uniform customColorUniform;

internal HashMap *skeletons;

internal uint32 wireframeRendererRefCount = 0;

internal UUID transformComponentID = {};
internal UUID modelComponentID = {};
internal UUID wireframeComponentID = {};
internal UUID animationComponentID = {};
internal UUID animatorComponentID = {};
internal UUID cameraComponentID = {};

internal CameraComponent *camera;
internal TransformComponent *cameraTransform;

extern real64 alpha;

extern uint32 animationSystemRefCount;

extern HashMap skeletonsMap;
extern HashMap animationReferences;

internal
void initWireframeRendererSystem(Scene *scene)
{
	if (wireframeRendererRefCount == 0)
	{
		LOG("Initializing wireframe renderer...\n");

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
			"hasAnimations",
			UNIFORM_BOOL,
			&hasAnimationsUniform);
		getUniform(
			shaderProgram,
			"boneTransforms",
			UNIFORM_MAT4,
			&boneTransformsUniform);

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

		LOG("Successfully initialized wireframe renderer\n");
	}

	wireframeRendererRefCount++;
}

internal
void beginWireframeRendererSystem(Scene *scene, real64 dt)
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

	if (animationSystemRefCount > 0)
	{
		skeletons = (HashMap*)hashMapGetData(skeletonsMap, &scene);
	}
}

internal
void runWireframeRendererSystem(Scene *scene, UUID entityID, real64 dt)
{
	if (!camera || !cameraTransform)
	{
		return;
	}

	ModelComponent *modelComponent = sceneGetComponentFromEntity(
		scene,
		entityID,
		modelComponentID);

	Model model = getModel(modelComponent->name);
	if (strlen(model.name.string) == 0)
	{
		return;
	}

	WireframeComponent *wireframeComponent = sceneGetComponentFromEntity(
		scene,
		entityID,
		wireframeComponentID);

	if (!wireframeComponent->visible)
	{
		return;
	}

	TransformComponent transform =
		*(TransformComponent*)sceneGetComponentFromEntity(
			scene,
			entityID,
			transformComponentID);

	kmVec3Mul(
		&transform.lastGlobalScale,
		&transform.lastGlobalScale,
		&wireframeComponent->scale);
	kmVec3Mul(
		&transform.globalScale,
		&transform.globalScale,
		&wireframeComponent->scale);

	kmMat4 worldMatrix = tGetInterpolatedTransformMatrix(
		&transform,
		alpha);

	setUniform(modelUniform, 1, &worldMatrix);

	AnimationComponent *animationComponent = sceneGetComponentFromEntity(
		scene,
		entityID,
		animationComponentID);

	AnimationReference *animationReference = NULL;
	HashMap *skeletonTransforms = NULL;

	if (animationSystemRefCount > 0 && animationComponent && skeletons)
	{
		AnimatorComponent *animator = sceneGetComponentFromEntity(
			scene,
			entityID,
			animatorComponentID);

		if (animator)
		{
			animationReference = hashMapGetData(animationReferences, &animator);
		}

		skeletonTransforms = hashMapGetData(
			*skeletons,
			&animationComponent->skeleton);
	}

	bool hasAnimations = animationReference && skeletonTransforms ?
		true : false;
	setUniform(hasAnimationsUniform, 1, &hasAnimations);

	if (hasAnimations)
	{
		kmMat4 boneMatrices[MAX_BONE_COUNT];
		for (uint32 i = 0; i < MAX_BONE_COUNT; i++)
		{
			kmMat4Identity(&boneMatrices[i]);
		}

		Skeleton *skeleton = &model.skeleton;
		for (uint32 i = 0; i < skeleton->numBoneOffsets; i++)
		{
			BoneOffset *boneOffset = &skeleton->boneOffsets[i];
			JointTransform *jointTransform = hashMapGetData(
				*skeletonTransforms,
				&boneOffset->name);

			if (jointTransform)
			{
				TransformComponent interpolatedJointTransform;
				tGetInterpolatedTransform(
					jointTransform->transform,
					&interpolatedJointTransform.globalPosition,
					&interpolatedJointTransform.globalRotation,
					&interpolatedJointTransform.globalScale,
					alpha);

				tConcatenateTransforms(
					&interpolatedJointTransform,
					&boneOffset->transform);
				boneMatrices[i] = tComposeMat4(
					&boneOffset->transform.globalPosition,
					&boneOffset->transform.globalRotation,
					&boneOffset->transform.globalScale);
			}
		}

		setUniform(
			boneTransformsUniform,
			MAX_BONE_COUNT,
			boneMatrices);
	}

	for (uint32 i = 0; i < model.numSubsets; i++)
	{
		Subset *subset = &model.subsets[i];
		Mesh *mesh = &subset->mesh;
		Material *material = &subset->material;
		Mask *mask = &subset->mask;

		glBindVertexArray(mesh->vertexArray);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);

		for (uint8 j = 0; j < NUM_VERTEX_ATTRIBUTES; j++)
		{
			glEnableVertexAttribArray(j);
		}

		bool customColor = true;
		setUniform(useCustomColorUniform, 1, &customColor);
		setUniform(customColorUniform, 1, &wireframeComponent->color);

		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		real32 lineWidth;
		glGetFloatv(GL_LINE_WIDTH, &lineWidth);
		glLineWidth(wireframeComponent->lineWidth);

		glDisable(GL_CULL_FACE);

		glDrawElements(
			GL_TRIANGLES,
			mesh->numIndices,
			GL_UNSIGNED_INT,
			NULL);

		logGLError(
			false,
			"Failed to draw wireframe for model (%s), subset (%s)",
			model.name.string,
			subset->name.string);

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

internal
void endWireframeRendererSystem(Scene *scene, real64 dt)
{
	if (!camera || !cameraTransform)
	{
		return;
	}

	glUseProgram(0);
}

internal
void shutdownWireframeRendererSystem(Scene *scene)
{
	if (--wireframeRendererRefCount == 0)
	{
		LOG("Shutting down wireframe renderer...\n");

		glDeleteProgram(shaderProgram);

		LOG("Successfully shut down wireframe renderer\n");
	}
}

System createWireframeRendererSystem(void)
{
	System system = {};

	transformComponentID = idFromName("transform");
	modelComponentID = idFromName("model");
	wireframeComponentID = idFromName("wireframe");
	animationComponentID = idFromName("animation");
	animatorComponentID = idFromName("animator");
	cameraComponentID = idFromName("camera");

	system.componentTypes = createList(sizeof(UUID));
	listPushFront(&system.componentTypes, &transformComponentID);
	listPushFront(&system.componentTypes, &modelComponentID);
	listPushFront(&system.componentTypes, &wireframeComponentID);

	system.init = &initWireframeRendererSystem;
	system.begin = &beginWireframeRendererSystem;
	system.run = &runWireframeRendererSystem;
	system.end = &endWireframeRendererSystem;
	system.shutdown = &shutdownWireframeRendererSystem;

	return system;
}