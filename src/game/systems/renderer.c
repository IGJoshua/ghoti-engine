#include "defines.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/model.h"
#include "asset_management/animation.h"
#include "asset_management/texture.h"

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

#define VERTEX_SHADER_FILE "resources/shaders/model.vert"
#define FRAGMENT_SHADER_FILE "resources/shaders/model.frag"

internal GLuint shaderProgram;

internal Uniform modelUniform;
internal Uniform viewUniform;
internal Uniform projectionUniform;

internal Uniform hasAnimationsUniform;
internal Uniform boneTransformsUniform;

internal Uniform cameraPositionUniform;

internal Uniform materialUniform;
internal Uniform materialValuesUniform;
internal Uniform materialMaskUniform;
internal Uniform opacityMaskUniform;
internal Uniform collectionMaterialUniform;
internal Uniform collectionMaterialValuesUniform;
internal Uniform grungeMaterialUniform;
internal Uniform grungeMaterialValuesUniform;
internal Uniform wearMaterialUniform;
internal Uniform wearMaterialValuesUniform;

internal Uniform useCustomColorUniform;

#define NUM_DIRECTIONAL_LIGHT_ATTRIBUTES 3
#define NUM_POINT_LIGHT_ATTRIBUTES 5
#define NUM_SPOTLIGHT_ATTRIBUTES 6

internal Uniform numDirectionalLightsUniform;
internal Uniform directionalLightUniforms[NUM_DIRECTIONAL_LIGHT_ATTRIBUTES];

internal Uniform numPointLightsUniform;
internal Uniform pointLightsUniforms
	[MAX_NUM_POINT_LIGHTS]
	[NUM_POINT_LIGHT_ATTRIBUTES];

internal Uniform numSpotlightsUniform;
internal Uniform spotlightsUniforms
	[MAX_NUM_SPOTLIGHTS]
	[NUM_SPOTLIGHT_ATTRIBUTES];

internal Uniform shadowDirectionalLightTransformUniform;

internal Uniform numDirectionalLightShadowMapsUniform;
internal Uniform directionalLightShadowMapUniform;
internal Uniform shadowDirectionalLightDirectionUniform;
internal Uniform shadowDirectionalLightBiasRangeUniform;

internal Uniform pointLightShadowMapsUniform;
internal Uniform shadowPointLightFarPlanesUniform;
internal Uniform shadowPointLightBiasUniform;
internal Uniform shadowPointLightDiskRadiusUniform;

internal HashMap *skeletons;

internal uint32 rendererRefCount = 0;

internal UUID transformComponentID = {};
internal UUID modelComponentID = {};
internal UUID animationComponentID = {};
internal UUID animatorComponentID = {};
internal UUID cameraComponentID = {};

internal CameraComponent *camera;
internal TransformComponent *cameraTransform;

extern Config config;
extern real64 alpha;

extern uint32 animationSystemRefCount;

extern HashMap skeletonsMap;
extern HashMap animationReferences;

extern uint32 numDirectionalLights;
extern DirectionalLight directionalLight;

extern uint32 numPointLights;
extern PointLight pointLights[MAX_NUM_POINT_LIGHTS];

extern uint32 numSpotlights;
extern Spotlight spotlights[MAX_NUM_SPOTLIGHTS];

extern uint32 numShadowDirectionalLights;
extern ShadowDirectionalLight shadowDirectionalLight;

extern uint32 numShadowPointLights;
extern ShadowPointLight shadowPointLights[MAX_NUM_POINT_LIGHTS];

internal
void initRendererSystem(Scene *scene)
{
	if (rendererRefCount == 0)
	{
		LOG("Initializing renderer...\n");

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
			"cameraPosition",
			UNIFORM_VEC3,
			&cameraPositionUniform);

		getUniform(
			shaderProgram,
			"material",
			UNIFORM_TEXTURE_2D,
			&materialUniform);
		getUniform(
			shaderProgram,
			"materialValues",
			UNIFORM_VEC3,
			&materialValuesUniform);
		getUniform(
			shaderProgram,
			"materialMask",
			UNIFORM_TEXTURE_2D,
			&materialMaskUniform);
		getUniform(
			shaderProgram,
			"opacityMask",
			UNIFORM_TEXTURE_2D,
			&opacityMaskUniform);
		getUniform(
			shaderProgram,
			"collectionMaterial",
			UNIFORM_TEXTURE_2D,
			&collectionMaterialUniform);
		getUniform(
			shaderProgram,
			"collectionMaterialValues",
			UNIFORM_VEC3,
			&collectionMaterialValuesUniform);
		getUniform(
			shaderProgram,
			"grungeMaterial",
			UNIFORM_TEXTURE_2D,
			&grungeMaterialUniform);
		getUniform(
			shaderProgram,
			"grungeMaterialValues",
			UNIFORM_VEC3,
			&grungeMaterialValuesUniform);
		getUniform(
			shaderProgram,
			"wearMaterial",
			UNIFORM_TEXTURE_2D,
			&wearMaterialUniform);
		getUniform(
			shaderProgram,
			"wearMaterialValues",
			UNIFORM_VEC3,
			&wearMaterialValuesUniform);

		getUniform(
			shaderProgram,
			"useCustomColor",
			UNIFORM_BOOL,
			&useCustomColorUniform);

		getUniform(
			shaderProgram,
			"numDirectionalLights",
			UNIFORM_UINT,
			&numDirectionalLightsUniform);

		uint8 attribute = 0;
		getUniform(
			shaderProgram,
			"directionalLight.color",
			UNIFORM_VEC3,
			&directionalLightUniforms[attribute++]);
		getUniform(
			shaderProgram,
			"directionalLight.ambient",
			UNIFORM_VEC3,
			&directionalLightUniforms[attribute++]);
		getUniform(
			shaderProgram,
			"directionalLight.direction",
			UNIFORM_VEC3,
			&directionalLightUniforms[attribute++]);

		getUniform(
			shaderProgram,
			"numPointLights",
			UNIFORM_UINT,
			&numPointLightsUniform);

		for (uint32 i = 0; i < MAX_NUM_POINT_LIGHTS; i++)
		{
			attribute = 0;

			char *uniformName = malloc(1024);
			sprintf(uniformName, "pointLights[%d].color", i);
			getUniform(
				shaderProgram,
				uniformName,
				UNIFORM_VEC3,
				&pointLightsUniforms[i][attribute++]);

			uniformName = malloc(1024);
			sprintf(uniformName, "pointLights[%d].ambient", i);
			getUniform(
				shaderProgram,
				uniformName,
				UNIFORM_VEC3,
				&pointLightsUniforms[i][attribute++]);

			uniformName = malloc(1024);
			sprintf(uniformName, "pointLights[%d].position", i);
			getUniform(
				shaderProgram,
				uniformName,
				UNIFORM_VEC3,
				&pointLightsUniforms[i][attribute++]);

			uniformName = malloc(1024);
			sprintf(uniformName, "pointLights[%d].radius", i);
			getUniform(
				shaderProgram,
				uniformName,
				UNIFORM_FLOAT,
				&pointLightsUniforms[i][attribute++]);

			uniformName = malloc(1024);
			sprintf(uniformName, "pointLights[%d].shadowIndex", i);
			getUniform(
				shaderProgram,
				uniformName,
				UNIFORM_INT,
				&pointLightsUniforms[i][attribute++]);
		}

		getUniform(
			shaderProgram,
			"numSpotlights",
			UNIFORM_UINT,
			&numSpotlightsUniform);

		for (uint32 i = 0; i < MAX_NUM_SPOTLIGHTS; i++)
		{
			attribute = 0;

			char *uniformName = malloc(1024);
			sprintf(uniformName, "spotlights[%d].color", i);
			getUniform(
				shaderProgram,
				uniformName,
				UNIFORM_VEC3,
				&spotlightsUniforms[i][attribute++]);

			uniformName = malloc(1024);
			sprintf(uniformName, "spotlights[%d].ambient", i);
			getUniform(
				shaderProgram,
				uniformName,
				UNIFORM_VEC3,
				&spotlightsUniforms[i][attribute++]);

			uniformName = malloc(1024);
			sprintf(uniformName, "spotlights[%d].position", i);
			getUniform(
				shaderProgram,
				uniformName,
				UNIFORM_VEC3,
				&spotlightsUniforms[i][attribute++]);

			uniformName = malloc(1024);
			sprintf(uniformName, "spotlights[%d].direction", i);
			getUniform(
				shaderProgram,
				uniformName,
				UNIFORM_VEC3,
				&spotlightsUniforms[i][attribute++]);

			uniformName = malloc(1024);
			sprintf(uniformName, "spotlights[%d].radius", i);
			getUniform(
				shaderProgram,
				uniformName,
				UNIFORM_FLOAT,
				&spotlightsUniforms[i][attribute++]);

			uniformName = malloc(1024);
			sprintf(uniformName, "spotlights[%d].size", i);
			getUniform(
				shaderProgram,
				uniformName,
				UNIFORM_VEC2,
				&spotlightsUniforms[i][attribute++]);
		}

		getUniform(
			shaderProgram,
			"shadowDirectionalLightTransform",
			UNIFORM_MAT4,
			&shadowDirectionalLightTransformUniform);

		getUniform(
			shaderProgram,
			"numDirectionalLightShadowMaps",
			UNIFORM_UINT,
			&numDirectionalLightShadowMapsUniform);
		getUniform(
			shaderProgram,
			"directionalLightShadowMap",
			UNIFORM_TEXTURE_2D,
			&directionalLightShadowMapUniform);
		getUniform(
			shaderProgram,
			"shadowDirectionalLightDirection",
			UNIFORM_VEC3,
			&shadowDirectionalLightDirectionUniform);
		getUniform(
			shaderProgram,
			"shadowDirectionalLightBiasRange",
			UNIFORM_VEC2,
			&shadowDirectionalLightBiasRangeUniform);

		getUniform(
			shaderProgram,
			"pointLightShadowMaps",
			UNIFORM_TEXTURE_CUBE_MAP,
			&pointLightShadowMapsUniform);
		getUniform(
			shaderProgram,
			"shadowPointLightFarPlanes",
			UNIFORM_FLOAT,
			&shadowPointLightFarPlanesUniform);
		getUniform(
			shaderProgram,
			"shadowPointLightBias",
			UNIFORM_FLOAT,
			&shadowPointLightBiasUniform);
		getUniform(
			shaderProgram,
			"shadowPointLightDiskRadius",
			UNIFORM_FLOAT,
			&shadowPointLightDiskRadiusUniform);

		LOG("Successfully initialized renderer\n");
	}

	rendererRefCount++;
}

internal
void beginRendererSystem(Scene *scene, real64 dt)
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

	kmVec3 cameraPosition;
	tGetInterpolatedTransform(
		cameraTransform,
		&cameraPosition,
		NULL,
		NULL,
		alpha);

	setUniform(cameraPositionUniform, 1, &cameraPosition);

	GLint textureIndex = 0;
	setUniform(directionalLightShadowMapUniform, 1, &textureIndex);
	textureIndex++;
	setTextureArrayUniform(
		&pointLightShadowMapsUniform,
		MAX_NUM_POINT_LIGHTS,
		&textureIndex);
	setMaterialUniform(&materialUniform, &textureIndex);
	setUniform(materialMaskUniform, 1, &textureIndex);
	textureIndex++;
	setUniform(opacityMaskUniform, 1, &textureIndex);
	textureIndex++;
	setMaterialUniform(&collectionMaterialUniform, &textureIndex);
	setMaterialUniform(&grungeMaterialUniform, &textureIndex);
	setMaterialUniform(&wearMaterialUniform, &textureIndex);

	setUniform(numDirectionalLightsUniform, 1, &numDirectionalLights);

	uint8 attribute = 0;
	if (numDirectionalLights == 1)
	{
		setUniform(
			directionalLightUniforms[attribute++],
			1,
			&directionalLight.color);
		setUniform(
			directionalLightUniforms[attribute++],
			1,
			&directionalLight.ambient);

		kmQuaternion directionalLightQuaternion;
		quaternionSlerp(
			&directionalLightQuaternion,
			&directionalLight.previousDirection,
			&directionalLight.direction,
			alpha);
		kmVec3 directionalLightDirection;
		kmQuaternionGetForwardVec3RH(
			&directionalLightDirection,
			&directionalLightQuaternion);

		setUniform(
			directionalLightUniforms[attribute++],
			1,
			&directionalLightDirection);
	}

	setUniform(numPointLightsUniform, 1, &numPointLights);

	for (uint32 i = 0; i < numPointLights; i++)
	{
		PointLight *pointLight = &pointLights[i];
		attribute = 0;

		setUniform(
			pointLightsUniforms[i][attribute++],
			1,
			&pointLight->color);
		setUniform(
			pointLightsUniforms[i][attribute++],
			1,
			&pointLight->ambient);

		kmVec3 pointLightPosition;
		kmVec3Lerp(
			&pointLightPosition,
			&pointLight->previousPosition,
			&pointLight->position,
			alpha);

		setUniform(
			pointLightsUniforms[i][attribute++],
			1,
			&pointLightPosition);
		setUniform(
			pointLightsUniforms[i][attribute++],
			1,
			&pointLight->radius);
		setUniform(
			pointLightsUniforms[i][attribute++],
			1,
			&pointLight->shadowIndex);
	}

	setUniform(numSpotlightsUniform, 1, &numSpotlights);

	for (uint32 i = 0; i < numSpotlights; i++)
	{
		Spotlight *spotlight = &spotlights[i];
		attribute = 0;

		setUniform(
			spotlightsUniforms[i][attribute++],
			1,
			&spotlight->color);
		setUniform(
			spotlightsUniforms[i][attribute++],
			1,
			&spotlight->ambient);

		kmVec3 spotlightPosition;
		kmVec3Lerp(
			&spotlightPosition,
			&spotlight->previousPosition,
			&spotlight->position,
			alpha);

		setUniform(
			spotlightsUniforms[i][attribute++],
			1,
			&spotlightPosition);

		kmQuaternion spotlightQuaternion;
		quaternionSlerp(
			&spotlightQuaternion,
			&spotlight->previousDirection,
			&spotlight->direction,
			alpha);
		kmVec3 spotlightDirection;
		kmQuaternionGetForwardVec3RH(
			&spotlightDirection,
			&spotlightQuaternion);

		setUniform(
			spotlightsUniforms[i][attribute++],
			1,
			&spotlightDirection);
		setUniform(
			spotlightsUniforms[i][attribute++],
			1,
			&spotlight->radius);
		setUniform(
			spotlightsUniforms[i][attribute++],
			1,
			&spotlight->size);
	}

	setUniform(
		shadowDirectionalLightTransformUniform,
		1,
		&shadowDirectionalLight.transform);
	setUniform(
		numDirectionalLightShadowMapsUniform,
		1,
		&numShadowDirectionalLights);
	setUniform(
		shadowDirectionalLightDirectionUniform,
		1,
		&shadowDirectionalLight.shaderDirection);
	setUniform(
		shadowDirectionalLightBiasRangeUniform,
		1,
		&config.graphicsConfig.directionalLightShadowBias);

	real32 pointLightFarPlanes[MAX_NUM_POINT_LIGHTS];
	for (uint32 i = 0; i < MAX_NUM_POINT_LIGHTS; i++)
	{
		pointLightFarPlanes[i] = shadowPointLights[i].farPlane;
	}

	setUniform(
		shadowPointLightFarPlanesUniform,
		MAX_NUM_POINT_LIGHTS,
		pointLightFarPlanes);
	setUniform(
		shadowPointLightBiasUniform,
		1,
		&config.graphicsConfig.pointLightShadowBias);
	setUniform(
		shadowPointLightDiskRadiusUniform,
		1,
		&config.graphicsConfig.pointLightPCFDiskRadius);

	if (animationSystemRefCount > 0)
	{
		skeletons = hashMapGetData(skeletonsMap, &scene);
	}
}

internal
void runRendererSystem(Scene *scene, UUID entityID, real64 dt)
{
	if (!camera || !cameraTransform)
	{
		return;
	}

	ModelComponent *modelComponent = sceneGetComponentFromEntity(
		scene,
		entityID,
		modelComponentID);

	if (!modelComponent->visible)
	{
		return;
	}

	Model model = getModel(modelComponent->name);
	if (strlen(model.name.string) == 0)
	{
		return;
	}

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

	TransformComponent *transform = sceneGetComponentFromEntity(
		scene,
		entityID,
		transformComponentID);

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
		Material *material = &subset->material;
		Mask *mask = &subset->mask;

		glBindVertexArray(mesh->vertexArray);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);

		for (uint8 j = 0; j < NUM_VERTEX_ATTRIBUTES; j++)
		{
			glEnableVertexAttribArray(j);
		}

		GLint textureIndex = 0;

		if (numShadowDirectionalLights > 0)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shadowDirectionalLight.shadowMap);
		}

		if (numShadowPointLights > 0)
		{
			GLuint pointLightShadowMaps[MAX_NUM_POINT_LIGHTS];
			for (uint32 i = 0; i < MAX_NUM_POINT_LIGHTS; i++)
			{
				pointLightShadowMaps[i] = shadowPointLights[i].shadowMap;
			}

			activateTextures(
				MAX_NUM_POINT_LIGHTS,
				GL_TEXTURE_CUBE_MAP,
				pointLightShadowMaps,
				&textureIndex);
		}

		textureIndex = 1 + MAX_NUM_POINT_LIGHTS;

		activateMaterialTextures(material, &textureIndex);
		activateTexture(model.materialTexture, &textureIndex);
		activateTexture(model.opacityTexture, &textureIndex);
		activateMaterialTextures(&mask->collectionMaterial, &textureIndex);
		activateMaterialTextures(&mask->grungeMaterial, &textureIndex);
		activateMaterialTextures(&mask->wearMaterial, &textureIndex);

		setMaterialValuesUniform(&materialValuesUniform, material);
		setMaterialValuesUniform(
			&collectionMaterialValuesUniform,
			&mask->collectionMaterial);
		setMaterialValuesUniform(
			&grungeMaterialValuesUniform,
			&mask->grungeMaterial);
		setMaterialValuesUniform(
			&wearMaterialValuesUniform,
			&mask->wearMaterial);

		if (material->doubleSided)
		{
			glDisable(GL_CULL_FACE);
		}

		bool useCustomColor = false;
		setUniform(useCustomColorUniform, 1, &useCustomColor);

		glDrawElements(
			GL_TRIANGLES,
			mesh->numIndices,
			GL_UNSIGNED_INT,
			NULL);

		logGLError(
			false,
			"Failed to draw model (%s), subset (%s)",
			model.name.string,
			subset->name.string);

		glEnable(GL_CULL_FACE);

		for (uint8 j = 0; j < MATERIAL_COMPONENT_TYPE_COUNT * 3 + 2; j++)
		{
			glActiveTexture(GL_TEXTURE0 + j);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		for (uint8 j = 0; j < NUM_VERTEX_ATTRIBUTES; j++)
		{
			glDisableVertexAttribArray(j);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
}

internal
void endRendererSystem(Scene *scene, real64 dt)
{
	if (!camera || !cameraTransform)
	{
		return;
	}

	glUseProgram(0);
}

internal
void shutdownRendererSystem(Scene *scene)
{
	if (--rendererRefCount == 0)
	{
		LOG("Shutting down renderer...\n");

		glDeleteProgram(shaderProgram);

		for (uint32 i = 0; i < MAX_NUM_POINT_LIGHTS; i++)
		{
			for (uint8 j = 0; j < NUM_POINT_LIGHT_ATTRIBUTES; j++)
			{
				free(pointLightsUniforms[i][j].name);
			}
		}

		for (uint32 i = 0; i < MAX_NUM_SPOTLIGHTS; i++)
		{
			for (uint8 j = 0; j < NUM_SPOTLIGHT_ATTRIBUTES; j++)
			{
				free(spotlightsUniforms[i][j].name);
			}
		}

		LOG("Successfully shut down renderer\n");
	}
}

System createRendererSystem(void)
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

	system.init = &initRendererSystem;
	system.begin = &beginRendererSystem;
	system.run = &runRendererSystem;
	system.end = &endRendererSystem;
	system.shutdown = &shutdownRendererSystem;

	return system;
}