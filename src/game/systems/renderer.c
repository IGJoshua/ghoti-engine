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
#define PBR_FRAGMENT_SHADER_FILE "resources/shaders/model_pbr.frag"
#define PBR_FALLBACK_FRAGMENT_SHADER_FILE \
	"resources/shaders/model_pbr_fallback.frag"

internal bool fallbackShaders;

internal GLuint shaderProgram;

internal Uniform modelUniform;
internal Uniform viewUniform;
internal Uniform projectionUniform;

internal Uniform hasAnimationsUniform;
internal Uniform boneTransformsUniform;

internal Uniform cameraPositionUniform;

internal Uniform materialActiveUniform;
internal Uniform materialUniform;
internal Uniform materialValuesUniform;

internal Uniform materialMaskUniform;
internal Uniform opacityMaskUniform;

internal Uniform collectionMaterialActiveUniform;
internal Uniform collectionMaterialUniform;
internal Uniform collectionMaterialValuesUniform;

internal Uniform grungeMaterialActiveUniform;
internal Uniform grungeMaterialUniform;
internal Uniform grungeMaterialValuesUniform;

internal Uniform wearMaterialActiveUniform;
internal Uniform wearMaterialUniform;
internal Uniform wearMaterialValuesUniform;

internal Uniform useCustomColorUniform;

internal Uniform irradianceMapUniform;
internal Uniform prefilterMapUniform;
internal Uniform brdfLUTUniform;

#define NUM_DIRECTIONAL_LIGHT_ATTRIBUTES 2
#define NUM_POINT_LIGHT_ATTRIBUTES 4
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
internal Uniform shadowSpotlightTransformsUniform;

internal Uniform numDirectionalLightShadowMapsUniform;
internal Uniform directionalLightShadowMapUniform;
internal Uniform shadowDirectionalLightBiasRangeUniform;

internal Uniform pointLightShadowMapsUniform;
internal Uniform shadowPointLightBiasUniform;
internal Uniform shadowPointLightDiskRadiusUniform;

internal Uniform spotlightShadowMapsUniform;
internal Uniform shadowSpotlightBiasRangeUniform;

internal HashMap *skeletons;

internal uint32 rendererRefCount = 0;

internal UUID transformComponentID = {};
internal UUID modelComponentID = {};
internal UUID animationComponentID = {};
internal UUID animatorComponentID = {};
internal UUID cameraComponentID = {};

internal CameraComponent *camera;
internal TransformComponent *cameraTransform;

#define BRDF_LUT_FILE "brdf_lut"

internal GLuint brdfLUT;

extern Config config;
extern real64 alpha;

extern uint32 animationSystemRefCount;

extern HashMap skeletonsMap;
extern HashMap animationReferences;

extern Cubemap currentCubemap;

extern uint32 numDirectionalLights;
extern DirectionalLight directionalLight;

extern uint32 numPointLights;
extern PointLight pointLights[MAX_NUM_POINT_LIGHTS];

extern uint32 numSpotlights;
extern Spotlight spotlights[MAX_NUM_SPOTLIGHTS];

extern uint32 numShadowDirectionalLights;
extern ShadowDirectionalLight shadowDirectionalLight;

extern uint32 numShadowPointLights;
extern ShadowPointLight shadowPointLights[MAX_NUM_SHADOW_POINT_LIGHTS];

extern uint32 numShadowSpotlights;
extern ShadowSpotlight shadowSpotlights[MAX_NUM_SHADOW_SPOTLIGHTS];

internal int32 loadBRDFLUT(void);

internal
void initRendererSystem(Scene *scene)
{
	if (rendererRefCount == 0)
	{
		LOG("Initializing renderer...\n");

		fallbackShaders =
			!GLEW_ARB_bindless_texture ||
			!GLEW_ARB_gpu_shader_int64;

		if (fallbackShaders)
		{
			LOG("WARNING: Required OpenGL extensions "
				"(GL_ARB_bindless_texture and GL_ARB_gpu_shader_int64) "
				"are not supported\n");
			LOG("Using fallback shaders\n");
		}

		loadBRDFLUT();

		const char *fragmentShader = PBR_FRAGMENT_SHADER_FILE;
		if (!config.graphicsConfig.pbr)
		{
			fragmentShader = FRAGMENT_SHADER_FILE;
			fallbackShaders = true;
		}
		else if (fallbackShaders)
		{
			fragmentShader = PBR_FALLBACK_FRAGMENT_SHADER_FILE;
		}

		createShaderProgram(
			VERTEX_SHADER_FILE,
			NULL,
			NULL,
			NULL,
			fragmentShader,
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
			"materialActive",
			UNIFORM_BOOL,
			&materialActiveUniform);
		getUniform(
			shaderProgram,
			"material",
			fallbackShaders ? UNIFORM_TEXTURE_2D : UNIFORM_TEXTURE_BINDLESS,
			&materialUniform);
		getUniform(
			shaderProgram,
			"materialValues",
			UNIFORM_VEC3,
			&materialValuesUniform);

		getUniform(
			shaderProgram,
			"materialMask",
			UNIFORM_TEXTURE_BINDLESS,
			&materialMaskUniform);
		getUniform(
			shaderProgram,
			"opacityMask",
			UNIFORM_TEXTURE_BINDLESS,
			&opacityMaskUniform);

		getUniform(
			shaderProgram,
			"collectionMaterialActive",
			UNIFORM_BOOL,
			&collectionMaterialActiveUniform);
		getUniform(
			shaderProgram,
			"collectionMaterial",
			UNIFORM_TEXTURE_BINDLESS,
			&collectionMaterialUniform);
		getUniform(
			shaderProgram,
			"collectionMaterialValues",
			UNIFORM_VEC3,
			&collectionMaterialValuesUniform);

		getUniform(
			shaderProgram,
			"grungeMaterialActive",
			UNIFORM_BOOL,
			&grungeMaterialActiveUniform);
		getUniform(
			shaderProgram,
			"grungeMaterial",
			UNIFORM_TEXTURE_BINDLESS,
			&grungeMaterialUniform);
		getUniform(
			shaderProgram,
			"grungeMaterialValues",
			UNIFORM_VEC3,
			&grungeMaterialValuesUniform);

		getUniform(
			shaderProgram,
			"wearMaterialActive",
			UNIFORM_BOOL,
			&wearMaterialActiveUniform);
		getUniform(
			shaderProgram,
			"wearMaterial",
			UNIFORM_TEXTURE_BINDLESS,
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
			"irradianceMap",
			UNIFORM_TEXTURE_CUBE_MAP,
			&irradianceMapUniform);
		getUniform(
			shaderProgram,
			"prefilterMap",
			UNIFORM_TEXTURE_CUBE_MAP,
			&prefilterMapUniform);
		getUniform(
			shaderProgram,
			"brdfLUT",
			UNIFORM_TEXTURE_2D,
			&brdfLUTUniform);

		getUniform(
			shaderProgram,
			"numDirectionalLights",
			UNIFORM_UINT,
			&numDirectionalLightsUniform);

		uint8 attribute = 0;
		getUniform(
			shaderProgram,
			"directionalLight.radiantFlux",
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
			sprintf(uniformName, "pointLights[%d].radiantFlux", i);
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
			sprintf(uniformName, "spotlights[%d].radiantFlux", i);
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

			uniformName = malloc(1024);
			sprintf(uniformName, "spotlights[%d].shadowIndex", i);
			getUniform(
				shaderProgram,
				uniformName,
				UNIFORM_INT,
				&spotlightsUniforms[i][attribute++]);
		}

		getUniform(
			shaderProgram,
			"shadowDirectionalLightTransform",
			UNIFORM_MAT4,
			&shadowDirectionalLightTransformUniform);
		getUniform(
			shaderProgram,
			"shadowSpotlightTransforms",
			UNIFORM_MAT4,
			&shadowSpotlightTransformsUniform);

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
			"shadowPointLightBias",
			UNIFORM_FLOAT,
			&shadowPointLightBiasUniform);
		getUniform(
			shaderProgram,
			"shadowPointLightDiskRadius",
			UNIFORM_FLOAT,
			&shadowPointLightDiskRadiusUniform);

		getUniform(
			shaderProgram,
			"spotlightShadowMaps",
			UNIFORM_TEXTURE_2D,
			&spotlightShadowMapsUniform);
		getUniform(
			shaderProgram,
			"shadowSpotlightBiasRange",
			UNIFORM_VEC2,
			&shadowSpotlightBiasRangeUniform);

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
		MAX_NUM_SHADOW_POINT_LIGHTS,
		&textureIndex);
	setTextureArrayUniform(
		&spotlightShadowMapsUniform,
		MAX_NUM_SHADOW_SPOTLIGHTS,
		&textureIndex);

	if (fallbackShaders)
	{
		setFallbackMaterialUniform(&materialUniform, &textureIndex);
	}

	if (config.graphicsConfig.pbr)
	{
		setUniform(irradianceMapUniform, 1, &textureIndex);
		textureIndex++;
		setUniform(prefilterMapUniform, 1, &textureIndex);
		textureIndex++;
		setUniform(brdfLUTUniform, 1, &textureIndex);
		textureIndex++;
	}

	setUniform(numDirectionalLightsUniform, 1, &numDirectionalLights);

	uint8 attribute = 0;
	if (numDirectionalLights == 1)
	{
		setUniform(
			directionalLightUniforms[attribute++],
			1,
			&directionalLight.radiantFlux);

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
			&pointLight->radiantFlux);

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
			&spotlight->radiantFlux);

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
		shadowDirectionalLightBiasRangeUniform,
		1,
		&config.graphicsConfig.directionalLightShadowBias);

	setUniform(
		shadowPointLightBiasUniform,
		1,
		&config.graphicsConfig.pointLightShadowBias);
	setUniform(
		shadowPointLightDiskRadiusUniform,
		1,
		&config.graphicsConfig.pointLightPCFDiskRadius);

	kmMat4 shadowSpotlightTransforms[MAX_NUM_SHADOW_SPOTLIGHTS];
	for (uint32 i = 0; i < MAX_NUM_SHADOW_SPOTLIGHTS; i++)
	{
		kmMat4Assign(
			&shadowSpotlightTransforms[i],
			&shadowSpotlights[i].transform);
	}

	setUniform(
		shadowSpotlightTransformsUniform,
		MAX_NUM_SHADOW_SPOTLIGHTS,
		shadowSpotlightTransforms);
	setUniform(
		shadowSpotlightBiasRangeUniform,
		1,
		&config.graphicsConfig.spotlightShadowBias);

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

		setMaterialActiveUniform(&materialActiveUniform, material);

		if (!fallbackShaders)
		{
			setMaterialActiveUniform(
				&collectionMaterialActiveUniform,
				&mask->collectionMaterial);
			setMaterialActiveUniform(
				&grungeMaterialActiveUniform,
				&mask->grungeMaterial);
			setMaterialActiveUniform(
				&wearMaterialActiveUniform,
				&mask->wearMaterial);
		}

		if (numShadowDirectionalLights > 0)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, shadowDirectionalLight.shadowMap);
		}

		GLint textureIndex = 1;

		if (numShadowPointLights > 0)
		{
			GLuint pointLightShadowMaps[MAX_NUM_SHADOW_POINT_LIGHTS];
			for (uint32 i = 0; i < numShadowPointLights; i++)
			{
				pointLightShadowMaps[i] = shadowPointLights[i].shadowMap;
			}

			activateTextures(
				numShadowPointLights,
				GL_TEXTURE_CUBE_MAP,
				pointLightShadowMaps,
				&textureIndex);
		}

		textureIndex = 1 + MAX_NUM_SHADOW_POINT_LIGHTS;

		if (numShadowSpotlights > 0)
		{
			GLuint spotlightShadowMaps[MAX_NUM_SHADOW_SPOTLIGHTS];
			for (uint32 i = 0; i < numShadowSpotlights; i++)
			{
				spotlightShadowMaps[i] = shadowSpotlights[i].shadowMap;
			}

			activateTextures(
				numShadowSpotlights,
				GL_TEXTURE_2D,
				spotlightShadowMaps,
				&textureIndex);
		}

		textureIndex =
			1 +
			MAX_NUM_SHADOW_POINT_LIGHTS +
			MAX_NUM_SHADOW_SPOTLIGHTS;

		if (fallbackShaders)
		{
			activateFallbackMaterialTextures(material, &textureIndex);
		}

		if (config.graphicsConfig.pbr)
		{
			glActiveTexture(GL_TEXTURE0 + textureIndex++);
			glBindTexture(GL_TEXTURE_CUBE_MAP, currentCubemap.irradianceID);

			glActiveTexture(GL_TEXTURE0 + textureIndex++);
			glBindTexture(GL_TEXTURE_CUBE_MAP, currentCubemap.prefilterID);

			glActiveTexture(GL_TEXTURE0 + textureIndex++);
			glBindTexture(GL_TEXTURE_2D, brdfLUT);
		}

		if (!fallbackShaders)
		{
			setMaterialUniform(&materialUniform, material);
			setBindlessTextureUniform(
				&materialMaskUniform,
				model.materialTexture);
			setBindlessTextureUniform(
				&opacityMaskUniform,
				model.opacityTexture);
			setMaterialUniform(
				&collectionMaterialUniform,
				&mask->collectionMaterial);
			setMaterialUniform(&grungeMaterialUniform, &mask->grungeMaterial);
			setMaterialUniform(&wearMaterialUniform, &mask->wearMaterial);
		}

		setMaterialValuesUniform(&materialValuesUniform, material);

		if (!fallbackShaders)
		{
			setMaterialValuesUniform(
				&collectionMaterialValuesUniform,
				&mask->collectionMaterial);
			setMaterialValuesUniform(
				&grungeMaterialValuesUniform,
				&mask->grungeMaterial);
			setMaterialValuesUniform(
				&wearMaterialValuesUniform,
				&mask->wearMaterial);
		}

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

		for (uint8 j = 0; j < 32; j++)
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

		glDeleteTextures(1, &brdfLUT);

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

int32 loadBRDFLUT(void)
{
	int32 error = 0;

	char *filename = getFullFilePath(
		BRDF_LUT_FILE,
		"hdr",
		"resources/cubemaps");

	HDRTextureData data;
	error = loadHDRTextureData(
		ASSET_LOG_TYPE_NONE,
		"BRDF LUT",
		NULL,
		filename,
		3,
		false,
		&data);

	free(filename);

	if (error != - 1)
	{
		glGenTextures(1, &brdfLUT);
		glBindTexture(GL_TEXTURE_2D, brdfLUT);

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RG16F,
			data.width,
			data.height,
			0,
			GL_RGB,
			GL_FLOAT,
			data.data);

		free(data.data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	return error;
}