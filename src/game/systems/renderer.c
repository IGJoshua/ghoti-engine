#include "defines.h"

#include "core/log.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/model.h"
#include "asset_management/texture.h"

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

internal GLuint shaderProgram;

internal Uniform modelUniform;
internal Uniform viewUniform;
internal Uniform projectionUniform;

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
internal Uniform customColorUniform;

internal uint32 rendererRefCount = 0;

internal UUID transformComponentID = {};
internal UUID modelComponentID = {};
internal UUID wireframeComponentID = {};
internal UUID cameraComponentID = {};

internal
void initRendererSystem(Scene *scene)
{
	if (rendererRefCount == 0)
	{
		createShaderProgram(
			"resources/shaders/base.vert",
			NULL,
			NULL,
			NULL,
			"resources/shaders/color.frag",
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
			"customColor",
			UNIFORM_VEC3,
			&customColorUniform);
	}

	rendererRefCount++;
}

extern real64 alpha;

internal
void beginRendererSystem(Scene *scene, real64 dt)
{
	glUseProgram(shaderProgram);

	if (cameraSetUniforms(
		scene,
		viewUniform,
		projectionUniform) == -1)
	{
		return;
	}

	GLint textureIndex = 0;
	setMaterialUniform(&materialUniform, &textureIndex);
	setUniform(materialMaskUniform, 1, &textureIndex);
	textureIndex++;
	setUniform(opacityMaskUniform, 1, &textureIndex);
	textureIndex++;
	setMaterialUniform(&collectionMaterialUniform, &textureIndex);
	setMaterialUniform(&grungeMaterialUniform, &textureIndex);
	setMaterialUniform(&wearMaterialUniform, &textureIndex);
}

extern real64 alpha;

internal
void runRendererSystem(Scene *scene, UUID entityID, real64 dt)
{
	if (!sceneGetComponentFromEntity(
		scene,
		scene->mainCamera,
		cameraComponentID))
	{
		return;
	}

	ModelComponent *modelComponent = sceneGetComponentFromEntity(
		scene,
		entityID,
		modelComponentID);

	WireframeComponent *wireframeComponent = sceneGetComponentFromEntity(
		scene,
		entityID,
		wireframeComponentID);

	Model *model = getModel(modelComponent->name);
	if (!model)
	{
		return;
	}

	bool wireframe = wireframeComponent && wireframeComponent->visible;
	if (!modelComponent->visible && !wireframe)
	{
		return;
	}

	TransformComponent *transform = sceneGetComponentFromEntity(
		scene,
		entityID,
		transformComponentID);

	kmMat4 worldMatrix = tGetInterpolatedTransformMatrix(
		transform,
		alpha);

	setUniform(modelUniform, 1, &worldMatrix);

	for (uint32 i = 0; i < model->numSubsets; i++)
	{
		Subset *subset = &model->subsets[i];
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
		activateMaterialTextures(material, &textureIndex);
		activateTexture(model->materialTexture, &textureIndex);
		activateTexture(model->opacityTexture, &textureIndex);
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

		if (modelComponent->visible)
		{
			bool useCustomColor = false;
			setUniform(useCustomColorUniform, 1, &useCustomColor);

			glDrawElements(
				GL_TRIANGLES,
				mesh->numIndices,
				GL_UNSIGNED_INT,
				NULL);

			logGLError(
				false,
				"Error when drawing model (%s), subset (%s)",
				model->name.string,
				subset->name.string);
		}

		if (wireframe)
		{
			setUniform(
				useCustomColorUniform,
				1,
				&wireframeComponent->customColor);
			setUniform(customColorUniform, 1, &wireframeComponent->color);

			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			real32 lineWidth;
			glGetFloatv(GL_LINE_WIDTH, &lineWidth);
			glLineWidth(wireframeComponent->lineWidth);

			glDrawElements(
				GL_TRIANGLES,
				mesh->numIndices,
				GL_UNSIGNED_INT,
				NULL);

			logGLError(
				false,
				"Error when drawing wireframe for model (%s), subset (%s)",
				model->name.string,
				subset->name.string);

			glLineWidth(lineWidth);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		if (material->doubleSided)
		{
			glEnable(GL_CULL_FACE);
		}

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
	glUseProgram(0);
}

internal
void shutdownRendererSystem(Scene *scene)
{
	if (--rendererRefCount == 0)
	{
		glDeleteProgram(shaderProgram);
	}
}

System createRendererSystem(void)
{
	System renderer = {};

	List componentList = createList(sizeof(UUID));

	transformComponentID = idFromName("transform");
	modelComponentID = idFromName("model");
	wireframeComponentID = idFromName("wireframe");
	cameraComponentID = idFromName("camera");

	listPushFront(&componentList, &transformComponentID);
	listPushFront(&componentList, &modelComponentID);

	renderer.componentTypes = componentList;

	renderer.init = &initRendererSystem;
	renderer.begin = &beginRendererSystem;
	renderer.run = &runRendererSystem;
	renderer.end = &endRendererSystem;
	renderer.shutdown = &shutdownRendererSystem;

	return renderer;
}