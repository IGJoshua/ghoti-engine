#include "defines.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/model.h"
#include "asset_management/animation.h"

#include "components/component_types.h"
#include "components/transform.h"
#include "components/camera.h"
#include "components/animation.h"
#include "components/animator.h"
#include "components/light.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/component.h"
#include "ECS/scene.h"

#include "math/math.h"

#include "renderer/renderer_types.h"
#include "renderer/renderer_utilities.h"
#include "renderer/shader.h"

#include <kazmath/mat4.h>
#include <kazmath/mat3.h>
#include <kazmath/quaternion.h>

#include <string.h>
#include <malloc.h>
#include <stdio.h>

#define VERTEX_SHADER_FILE "resources/shaders/shadows.vert"
#define GEOMETRY_SHADER_FILE "resources/shaders/shadows.geom"
#define FRAGMENT_SHADER_FILE "resources/shaders/shadows.frag"

internal GLuint shaderProgram;

internal Uniform modelUniform;

internal Uniform hasAnimationsUniform;
internal Uniform boneTransformsUniform;

internal Uniform lightTransformsUniform;
internal Uniform lightPositionUniform;
internal Uniform farPlaneUniform;

internal HashMap *skeletons;

uint32 shadowsSystemRefCount = 0;

internal UUID transformComponentID = {};
internal UUID modelComponentID = {};
internal UUID animationComponentID = {};
internal UUID animatorComponentID = {};
internal UUID lightComponentID = {};

uint32 numShadowPointLights = 0;
ShadowPointLight shadowPointLights[MAX_NUM_SHADOW_POINT_LIGHTS];

#define SHADOW_MAP_WIDTH 1024
#define SHADOW_MAP_HEIGHT 1024

internal GLuint shadowMapFramebuffer;

extern real64 alpha;

extern int32 viewportWidth;
extern int32 viewportHeight;

extern uint32 animationSystemRefCount;
extern uint32 postProcessingSystemRefCount;

extern GLuint screenFramebufferMSAA;

extern HashMap skeletonsMap;
extern HashMap animationReferences;

internal void drawAllShadows(Scene *scene);
internal void drawShadows(
	ModelComponent *modelComponent,
	TransformComponent *transform,
	AnimationComponent *animationComponent,
	AnimatorComponent *animator);

internal void initShadowsSystem(Scene *scene)
{
	if (shadowsSystemRefCount == 0)
	{
		LOG("Initializing shadows system...\n");

		createShaderProgram(
			VERTEX_SHADER_FILE,
			NULL,
			NULL,
			GEOMETRY_SHADER_FILE,
			FRAGMENT_SHADER_FILE,
			NULL,
			&shaderProgram);

		getUniform(shaderProgram, "model", UNIFORM_MAT4, &modelUniform);

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
			"lightTransforms",
			UNIFORM_MAT4,
			&lightTransformsUniform);
		getUniform(
			shaderProgram,
			"lightPosition",
			UNIFORM_VEC3,
			&lightPositionUniform);
		getUniform(
			shaderProgram,
			"farPlane",
			UNIFORM_FLOAT,
			&farPlaneUniform);

		for (uint32 i = 0; i < MAX_NUM_SHADOW_POINT_LIGHTS; i++)
		{
			glGenTextures(1, &shadowPointLights[i].shadowMap);
			glBindTexture(GL_TEXTURE_CUBE_MAP, shadowPointLights[i].shadowMap);

			for (uint8 i = 0; i < 6; i++)
			{
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0,
					GL_DEPTH_COMPONENT,
					SHADOW_MAP_WIDTH,
					SHADOW_MAP_HEIGHT,
					0,
					GL_DEPTH_COMPONENT,
					GL_FLOAT,
					NULL);
			}

			glTexParameteri(
				GL_TEXTURE_CUBE_MAP,
				GL_TEXTURE_MAG_FILTER,
				GL_NEAREST);
			glTexParameteri(
				GL_TEXTURE_CUBE_MAP,
				GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
			glTexParameteri(
				GL_TEXTURE_2D,
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
		}

		glGenFramebuffers(1, &shadowMapFramebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFramebuffer);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		LOG("Successfully initialized shadows system\n");
	}

	shadowsSystemRefCount++;
}

internal void beginShadowsSystem(Scene *scene, real64 dt)
{
	if (numShadowPointLights == 0)
	{
		return;
	}

	glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFramebuffer);

	glUseProgram(shaderProgram);

	for (uint32 i = 0; i < numShadowPointLights; i++)
	{
		ShadowPointLight *shadowPointLight = &shadowPointLights[i];

		glFramebufferTexture(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			shadowPointLight->shadowMap,
			0);

		glClear(GL_DEPTH_BUFFER_BIT);

		kmMat4 lightProjection;
		kmMat4PerspectiveProjection(
			&lightProjection,
			90.0f,
			(real32)SHADOW_MAP_WIDTH / (real32)SHADOW_MAP_HEIGHT,
			0.01f,
			shadowPointLight->farPlane);

		TransformComponent lightTransform = {};
		kmVec3Assign(
			&lightTransform.lastGlobalPosition,
			&shadowPointLight->previousPosition);
		kmVec3Assign(
			&lightTransform.globalPosition,
			&shadowPointLight->position);

		kmVec3 lightPosition;
		tGetInterpolatedTransform(
			&lightTransform,
			&lightPosition,
			NULL,
			NULL,
			alpha);

		kmQuaternion lightRotations[6];

		// Right Face
		kmQuaternionLookRotation(
			&lightRotations[0],
			&KM_VEC3_POS_X,
			&KM_VEC3_NEG_Y);

		// Left Face
		kmQuaternionLookRotation(
			&lightRotations[1],
			&KM_VEC3_NEG_X,
			&KM_VEC3_NEG_Y);

		// Top Face
		kmQuaternionLookRotation(
			&lightRotations[2],
			&KM_VEC3_POS_Y,
			&KM_VEC3_POS_Z);

		// Bottom Face
		kmQuaternionLookRotation(
			&lightRotations[3],
			&KM_VEC3_NEG_Y,
			&KM_VEC3_NEG_Z);

		// Front Face
		kmQuaternionLookRotation(
			&lightRotations[4],
			&KM_VEC3_POS_Z,
			&KM_VEC3_NEG_Y);

		// Back Face
		kmQuaternionLookRotation(
			&lightRotations[5],
			&KM_VEC3_NEG_Z,
			&KM_VEC3_NEG_Y);

		for (uint8 i = 0; i < 6; i++)
		{
			kmQuaternionInverse(&lightRotations[i], &lightRotations[i]);
		}

		kmVec3 lightScale;
		kmVec3Fill(&lightScale, 1.0f, 1.0f, 1.0f);

		kmMat4 lightTransforms[6];

		for (uint8 i = 0; i < 6; i++)
		{
			kmMat4 lightView = tComposeMat4(
				&lightPosition,
				&lightRotations[i],
				&lightScale);
			kmMat4Inverse(&lightView, &lightView);
			kmMat4Multiply(&lightTransforms[i], &lightProjection, &lightView);
		}

		setUniform(lightTransformsUniform, 6, lightTransforms);
		setUniform(lightPositionUniform, 1, &lightPosition);
		setUniform(farPlaneUniform, 1, &shadowPointLight->farPlane);

		drawAllShadows(scene);
	}

	if (animationSystemRefCount > 0)
	{
		skeletons = hashMapGetData(skeletonsMap, &scene);
	}
}

internal void endShadowsSystem(Scene *scene, real64 dt)
{
	if (numShadowPointLights == 0)
	{
		return;
	}

	glUseProgram(0);
	glViewport(0, 0, viewportWidth, viewportHeight);

	if (postProcessingSystemRefCount > 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, screenFramebufferMSAA);
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}

internal void shutdownShadowsSystem(Scene *scene)
{
	if (--shadowsSystemRefCount == 0)
	{
		LOG("Shutting down shadows system...\n");

		glDeleteProgram(shaderProgram);
		glDeleteFramebuffers(1, &shadowMapFramebuffer);

		for (uint32 i = 0; i < MAX_NUM_SHADOW_POINT_LIGHTS; i++)
		{
			glDeleteTextures(1, &shadowPointLights[i].shadowMap);
		}

		LOG("Successfully shut down shadows system\n");
	}
}

System createShadowsSystem(void)
{
	System system = {};

	transformComponentID = idFromName("transform");
	modelComponentID = idFromName("model");
	animationComponentID = idFromName("animation");
	animatorComponentID = idFromName("animator");
	lightComponentID = idFromName("light");

	system.componentTypes = createList(sizeof(UUID));
	listPushFront(&system.componentTypes, &transformComponentID);
	listPushFront(&system.componentTypes, &modelComponentID);

	system.init = &initShadowsSystem;
	system.begin = &beginShadowsSystem;
	system.end = &endShadowsSystem;
	system.shutdown = &shutdownShadowsSystem;

	return system;
}

void drawAllShadows(Scene *scene)
{
	ComponentDataTable *modelComponents =
		*(ComponentDataTable**)hashMapGetData(
			scene->componentTypes,
			&modelComponentID);

	for (ComponentDataTableIterator itr = cdtGetIterator(modelComponents);
		 !cdtIteratorAtEnd(itr);
		 cdtMoveIterator(&itr))
	{
		ModelComponent *modelComponent = cdtIteratorGetData(itr);

		UUID entity = cdtIteratorGetUUID(itr);

		TransformComponent *transformComponent =
			sceneGetComponentFromEntity(
				scene,
				entity,
				transformComponentID);

		if (!transformComponent)
		{
			continue;
		}

		AnimationComponent *animationComponent =
			sceneGetComponentFromEntity(
				scene,
				entity,
				animationComponentID);
		AnimatorComponent *animator =
			sceneGetComponentFromEntity(
				scene,
				entity,
				animatorComponentID);

		drawShadows(
			modelComponent,
			transformComponent,
			animationComponent,
			animator);
	}
}

void drawShadows(
	ModelComponent *modelComponent,
	TransformComponent *transform,
	AnimationComponent *animationComponent,
	AnimatorComponent *animator)
{
	if (!modelComponent->visible)
	{
		return;
	}

	Model model = getModel(modelComponent->name);
	if (strlen(model.name.string) == 0)
	{
		return;
	}

	AnimationReference *animationReference = NULL;
	HashMap *skeletonTransforms = NULL;

	if (animationSystemRefCount > 0 && animationComponent && skeletons)
	{
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

	kmMat4 worldMatrix = tGetInterpolatedTransformMatrix(
		transform,
		alpha);

	if (hasAnimations)
	{
		kmMat4Identity(&worldMatrix);

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

	setUniform(modelUniform, 1, &worldMatrix);

	for (uint32 i = 0; i < model.numSubsets; i++)
	{
		Subset *subset = &model.subsets[i];
		Mesh *mesh = &subset->mesh;

		glBindVertexArray(mesh->vertexArray);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);

		for (uint8 j = 0; j < NUM_VERTEX_ATTRIBUTES; j++)
		{
			glEnableVertexAttribArray(j);
		}

		glDrawElements(
			GL_TRIANGLES,
			mesh->numIndices,
			GL_UNSIGNED_INT,
			NULL);

		logGLError(
			false,
			"Failed to draw render shadows for model (%s)",
			model.name.string);

		for (uint8 j = 0; j < NUM_VERTEX_ATTRIBUTES; j++)
		{
			glDisableVertexAttribArray(j);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
}