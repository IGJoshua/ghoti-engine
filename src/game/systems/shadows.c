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
#include "core/config.h"

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

#define DIRECTIONAL_SHADOWS_VERTEX_SHADER_FILE \
	"resources/shaders/directional_shadows.vert"
#define DIRECTIONAL_SHADOWS_FRAGMENT_SHADER_FILE \
	"resources/shaders/directional_shadows.frag"

typedef struct directional_shadows_shader_t
{
	GLuint shaderProgram;
	Uniform modelUniform;
	Uniform hasAnimationsUniform;
	Uniform boneTransformsUniform;
	Uniform lightTransformUniform;
} DirectionalShadowsShader;

#define POINT_SHADOWS_VERTEX_SHADER_FILE \
	"resources/shaders/point_shadows.vert"
#define POINT_SHADOWS_GEOMETRY_SHADER_FILE \
	"resources/shaders/point_shadows.geom"
#define POINT_SHADOWS_FRAGMENT_SHADER_FILE \
	"resources/shaders/point_shadows.frag"

typedef struct point_shadows_shader_t
{
	GLuint shaderProgram;
	Uniform modelUniform;
	Uniform hasAnimationsUniform;
	Uniform boneTransformsUniform;
	Uniform lightTransformsUniform;
	Uniform lightPositionUniform;
	Uniform farPlaneUniform;
} PointShadowsShader;

DirectionalShadowsShader shadowDirectionalLightShader;
DirectionalShadowsShader shadowSpotlightsShader;
PointShadowsShader shadowPointLightsShader;

internal HashMap *skeletons;

uint32 shadowsSystemRefCount = 0;

internal UUID transformComponentID = {};
internal UUID modelComponentID = {};
internal UUID animationComponentID = {};
internal UUID animatorComponentID = {};
internal UUID cameraComponentID = {};

uint32 numShadowDirectionalLights = 0;
ShadowDirectionalLight shadowDirectionalLight;

uint32 numShadowPointLights = 0;
ShadowPointLight shadowPointLights[MAX_NUM_SHADOW_POINT_LIGHTS];

uint32 numShadowSpotlights = 0;
ShadowSpotlight shadowSpotlights[MAX_NUM_SHADOW_SPOTLIGHTS];

internal GLuint shadowMapFramebuffer;

extern Config config;
extern real64 alpha;

extern int32 viewportWidth;
extern int32 viewportHeight;

extern uint32 animationSystemRefCount;
extern uint32 postProcessingSystemRefCount;

extern GLuint screenFramebufferMSAA;

extern HashMap skeletonsMap;
extern HashMap animationReferences;

internal void initializeShadowDirectionalLightShader(void);
internal void drawShadowDirectionalLight(Scene *scene);

internal void initializeShadowPointLightsShader(void);
internal void drawShadowPointLights(Scene *scene);

internal void initializeShadowSpotlightsShader(void);
internal void drawShadowSpotlights(Scene *scene);

internal void drawAllShadows(
	Scene *scene,
	Uniform *modelUniform,
	Uniform *hasAnimationsUniform,
	Uniform *boneTransformsUniform);
internal void drawShadows(
	ModelComponent *modelComponent,
	TransformComponent *transform,
	AnimationComponent *animationComponent,
	AnimatorComponent *animator,
	Uniform *modelUniform,
	Uniform *hasAnimationsUniform,
	Uniform *boneTransformsUniform);

internal void initShadowsSystem(Scene *scene)
{
	if (shadowsSystemRefCount == 0)
	{
		LOG("Initializing shadows system...\n");

		initializeShadowDirectionalLightShader();
		initializeShadowPointLightsShader();
		initializeShadowSpotlightsShader();

		real32 white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glGenTextures(1, &shadowDirectionalLight.shadowMap);
		glBindTexture(GL_TEXTURE_2D, shadowDirectionalLight.shadowMap);

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_DEPTH_COMPONENT,
			config.graphicsConfig.shadowMapResolution,
			config.graphicsConfig.shadowMapResolution,
			0,
			GL_DEPTH_COMPONENT,
			GL_FLOAT,
			NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, white);

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
					config.graphicsConfig.shadowMapResolution,
					config.graphicsConfig.shadowMapResolution,
					0,
					GL_DEPTH_COMPONENT,
					GL_FLOAT,
					NULL);
			}

			glTexParameteri(
				GL_TEXTURE_CUBE_MAP,
				GL_TEXTURE_MAG_FILTER,
				GL_LINEAR);
			glTexParameteri(
				GL_TEXTURE_CUBE_MAP,
				GL_TEXTURE_MIN_FILTER,
				GL_LINEAR);
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

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		for (uint32 i = 0; i < MAX_NUM_SHADOW_SPOTLIGHTS; i++)
		{
			glGenTextures(1, &shadowSpotlights[i].shadowMap);
			glBindTexture(GL_TEXTURE_2D, shadowSpotlights[i].shadowMap);

			glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_DEPTH_COMPONENT,
				config.graphicsConfig.shadowMapResolution,
				config.graphicsConfig.shadowMapResolution,
				0,
				GL_DEPTH_COMPONENT,
				GL_FLOAT,
				NULL);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(
				GL_TEXTURE_2D,
				GL_TEXTURE_WRAP_S,
				GL_CLAMP_TO_BORDER);
			glTexParameteri(
				GL_TEXTURE_2D,
				GL_TEXTURE_WRAP_T,
				GL_CLAMP_TO_BORDER);
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, white);
		}

		glBindTexture(GL_TEXTURE_2D, 0);

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
	if (numShadowDirectionalLights == 0 &&
		numShadowPointLights == 0 &&
		numShadowSpotlights == 0)
	{
		return;
	}

	glViewport(
		0,
		0,
		config.graphicsConfig.shadowMapResolution,
		config.graphicsConfig.shadowMapResolution);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFramebuffer);

	if (animationSystemRefCount > 0)
	{
		skeletons = hashMapGetData(skeletonsMap, &scene);
	}

	drawShadowDirectionalLight(scene);
	drawShadowPointLights(scene);
	drawShadowSpotlights(scene);
}

internal void endShadowsSystem(Scene *scene, real64 dt)
{
	if (numShadowDirectionalLights == 0 &&
		numShadowPointLights == 0 &&
		numShadowSpotlights == 0)
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

		glDeleteProgram(shadowPointLightsShader.shaderProgram);
		glDeleteProgram(shadowDirectionalLightShader.shaderProgram);
		glDeleteProgram(shadowSpotlightsShader.shaderProgram);

		glDeleteTextures(1, &shadowDirectionalLight.shadowMap);

		for (uint32 i = 0; i < MAX_NUM_SHADOW_POINT_LIGHTS; i++)
		{
			glDeleteTextures(1, &shadowPointLights[i].shadowMap);
		}

		for (uint32 i = 0; i < MAX_NUM_SHADOW_SPOTLIGHTS; i++)
		{
			glDeleteTextures(1, &shadowSpotlights[i].shadowMap);
		}

		glDeleteFramebuffers(1, &shadowMapFramebuffer);

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
	cameraComponentID = idFromName("camera");

	system.componentTypes = createList(sizeof(UUID));
	listPushFront(&system.componentTypes, &transformComponentID);
	listPushFront(&system.componentTypes, &modelComponentID);

	system.init = &initShadowsSystem;
	system.begin = &beginShadowsSystem;
	system.end = &endShadowsSystem;
	system.shutdown = &shutdownShadowsSystem;

	return system;
}

void initializeShadowDirectionalLightShader(void)
{
	createShaderProgram(
		DIRECTIONAL_SHADOWS_VERTEX_SHADER_FILE,
		NULL,
		NULL,
		NULL,
		DIRECTIONAL_SHADOWS_FRAGMENT_SHADER_FILE,
		NULL,
		&shadowDirectionalLightShader.shaderProgram);

	getUniform(
		shadowDirectionalLightShader.shaderProgram,
		"model",
		UNIFORM_MAT4,
		&shadowDirectionalLightShader.modelUniform);

	getUniform(
		shadowDirectionalLightShader.shaderProgram,
		"hasAnimations",
		UNIFORM_BOOL,
		&shadowDirectionalLightShader.hasAnimationsUniform);
	getUniform(
		shadowDirectionalLightShader.shaderProgram,
		"boneTransforms",
		UNIFORM_MAT4,
		&shadowDirectionalLightShader.boneTransformsUniform);

	getUniform(
		shadowDirectionalLightShader.shaderProgram,
		"lightTransform",
		UNIFORM_MAT4,
		&shadowDirectionalLightShader.lightTransformUniform);
}

void drawShadowDirectionalLight(Scene *scene)
{
	if (numShadowDirectionalLights == 0)
	{
		return;
	}

	TransformComponent *cameraTransform = sceneGetComponentFromEntity(
		scene,
		scene->mainCamera,
		transformComponentID);
	CameraComponent *camera = sceneGetComponentFromEntity(
		scene,
		scene->mainCamera,
		cameraComponentID);

	if (!cameraTransform || !camera)
	{
		return;
	}

	glUseProgram(shadowDirectionalLightShader.shaderProgram);

	glFramebufferTexture(
		GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		shadowDirectionalLight.shadowMap,
		0);

	glClear(GL_DEPTH_BUFFER_BIT);

	// TODO: Properly build light frustum
	kmMat4 lightProjection;
	kmMat4OrthographicProjection(
		&lightProjection,
		-20.0f,
		20.0f,
		-20.0f,
		20.0f,
		-20.0f,
		20.0f);

	kmQuaternion lightDirection;
	quaternionSlerp(
		&lightDirection,
		&shadowDirectionalLight.previousDirection,
		&shadowDirectionalLight.direction,
		alpha);

	kmVec3 directionVector;
	kmQuaternionGetForwardVec3RH(
		&directionVector,
		&lightDirection);

	kmVec3 cameraPosition;
	tGetInterpolatedTransform(
		cameraTransform,
		&cameraPosition,
		NULL,
		NULL,
		alpha);

	// TODO: Properly build light frustum
	kmVec3 lightPosition;
	kmVec3Scale(&lightPosition, &directionVector, -5.0f);
	kmVec3Add(&lightPosition, &lightPosition, &cameraPosition);

	kmVec3 lightScale;
	kmVec3Fill(&lightScale, 1.0f, 1.0f, 1.0f);

	kmMat4 lightView = tComposeMat4(
		&lightPosition,
		&lightDirection,
		&lightScale);
	kmMat4Inverse(&lightView, &lightView);

	kmMat4Multiply(
		&shadowDirectionalLight.transform,
		&lightProjection,
		&lightView);

	setUniform(
		shadowDirectionalLightShader.lightTransformUniform,
		1,
		&shadowDirectionalLight.transform);

	drawAllShadows(
		scene,
		&shadowDirectionalLightShader.modelUniform,
		&shadowDirectionalLightShader.hasAnimationsUniform,
		&shadowDirectionalLightShader.boneTransformsUniform);
}

void initializeShadowPointLightsShader(void)
{
	createShaderProgram(
		POINT_SHADOWS_VERTEX_SHADER_FILE,
		NULL,
		NULL,
		POINT_SHADOWS_GEOMETRY_SHADER_FILE,
		POINT_SHADOWS_FRAGMENT_SHADER_FILE,
		NULL,
		&shadowPointLightsShader.shaderProgram);

	getUniform(
		shadowPointLightsShader.shaderProgram,
		"model",
		UNIFORM_MAT4,
		&shadowPointLightsShader.modelUniform);

	getUniform(
		shadowPointLightsShader.shaderProgram,
		"hasAnimations",
		UNIFORM_BOOL,
		&shadowPointLightsShader.hasAnimationsUniform);
	getUniform(
		shadowPointLightsShader.shaderProgram,
		"boneTransforms",
		UNIFORM_MAT4,
		&shadowPointLightsShader.boneTransformsUniform);

	getUniform(
		shadowPointLightsShader.shaderProgram,
		"lightTransforms",
		UNIFORM_MAT4,
		&shadowPointLightsShader.lightTransformsUniform);
	getUniform(
		shadowPointLightsShader.shaderProgram,
		"lightPosition",
		UNIFORM_VEC3,
		&shadowPointLightsShader.lightPositionUniform);
	getUniform(
		shadowPointLightsShader.shaderProgram,
		"farPlane",
		UNIFORM_FLOAT,
		&shadowPointLightsShader.farPlaneUniform);
}

void drawShadowPointLights(Scene *scene)
{
	glUseProgram(shadowPointLightsShader.shaderProgram);

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
			1.0f,
			0.01f,
			shadowPointLight->farPlane);

		kmVec3 lightPosition;
		kmVec3Lerp(
			&lightPosition,
			&shadowPointLight->previousPosition,
			&shadowPointLight->position,
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

		setUniform(
			shadowPointLightsShader.lightTransformsUniform,
			6,
			lightTransforms);
		setUniform(
			shadowPointLightsShader.lightPositionUniform,
			1,
			&lightPosition);
		setUniform(
			shadowPointLightsShader.farPlaneUniform,
			1,
			&shadowPointLight->farPlane);

		drawAllShadows(
			scene,
			&shadowPointLightsShader.modelUniform,
			&shadowPointLightsShader.hasAnimationsUniform,
			&shadowPointLightsShader.boneTransformsUniform);
	}
}

void initializeShadowSpotlightsShader(void)
{
	createShaderProgram(
		DIRECTIONAL_SHADOWS_VERTEX_SHADER_FILE,
		NULL,
		NULL,
		NULL,
		DIRECTIONAL_SHADOWS_FRAGMENT_SHADER_FILE,
		NULL,
		&shadowSpotlightsShader.shaderProgram);

	getUniform(
		shadowSpotlightsShader.shaderProgram,
		"model",
		UNIFORM_MAT4,
		&shadowSpotlightsShader.modelUniform);

	getUniform(
		shadowSpotlightsShader.shaderProgram,
		"hasAnimations",
		UNIFORM_BOOL,
		&shadowSpotlightsShader.hasAnimationsUniform);
	getUniform(
		shadowSpotlightsShader.shaderProgram,
		"boneTransforms",
		UNIFORM_MAT4,
		&shadowSpotlightsShader.boneTransformsUniform);

	getUniform(
		shadowSpotlightsShader.shaderProgram,
		"lightTransform",
		UNIFORM_MAT4,
		&shadowSpotlightsShader.lightTransformUniform);
}

void drawShadowSpotlights(Scene *scene)
{
	glUseProgram(shadowSpotlightsShader.shaderProgram);

	for (uint32 i = 0; i < numShadowSpotlights; i++)
	{
		ShadowSpotlight *shadowSpotlight = &shadowSpotlights[i];

		glFramebufferTexture(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			shadowSpotlight->shadowMap,
			0);

		glClear(GL_DEPTH_BUFFER_BIT);

		kmMat4 lightProjection;
		kmMat4PerspectiveProjection(
			&lightProjection,
			shadowSpotlight->fov,
			1.0f,
			0.01f,
			shadowSpotlight->farPlane);

		kmVec3 lightPosition;
		kmVec3Lerp(
			&lightPosition,
			&shadowSpotlight->previousPosition,
			&shadowSpotlight->position,
			alpha);

		kmQuaternion lightDirection;
		quaternionSlerp(
			&lightDirection,
			&shadowSpotlight->previousDirection,
			&shadowSpotlight->direction,
			alpha);

		kmVec3 lightScale;
		kmVec3Fill(&lightScale, 1.0f, 1.0f, 1.0f);

		kmMat4 lightView = tComposeMat4(
			&lightPosition,
			&lightDirection,
			&lightScale);
		kmMat4Inverse(&lightView, &lightView);

		kmMat4Multiply(
			&shadowSpotlight->transform,
			&lightProjection,
			&lightView);

		setUniform(
			shadowSpotlightsShader.lightTransformUniform,
			1,
			&shadowSpotlight->transform);

		drawAllShadows(
			scene,
			&shadowSpotlightsShader.modelUniform,
			&shadowSpotlightsShader.hasAnimationsUniform,
			&shadowSpotlightsShader.boneTransformsUniform);
	}
}

void drawAllShadows(
	Scene *scene,
	Uniform *modelUniform,
	Uniform *hasAnimationsUniform,
	Uniform *boneTransformsUniform)
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
			animator,
			modelUniform,
			hasAnimationsUniform,
			boneTransformsUniform);
	}
}

void drawShadows(
	ModelComponent *modelComponent,
	TransformComponent *transform,
	AnimationComponent *animationComponent,
	AnimatorComponent *animator,
	Uniform *modelUniform,
	Uniform *hasAnimationsUniform,
	Uniform *boneTransformsUniform)
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
	setUniform(*hasAnimationsUniform, 1, &hasAnimations);

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
			*boneTransformsUniform,
			MAX_BONE_COUNT,
			boneMatrices);
	}

	setUniform(*modelUniform, 1, &worldMatrix);

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