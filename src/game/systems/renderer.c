#include "defines.h"

#include "core/log.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/model.h"
#include "asset_management/texture.h"

#include "renderer/renderer_types.h"
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

internal Shader vertexShader;
internal Shader fragmentShader;

internal ShaderPipeline pipeline;

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

internal bool rendererActive;

internal UUID transformComponentID = {};
internal UUID modelComponentID = {};
internal UUID wireframeComponentID = {};
internal UUID cameraComponentID = {};

internal int32 setMaterialUniform(Uniform *uniform, GLint *textureIndex);
int32 setMaterialValuesUniform(Uniform *uniform, Material *material);
void activateMaterialTextures(Material *material, GLint *textureIndex);
void activateTexture(UUID name, GLint *textureIndex);

internal
void initRendererSystem(Scene *scene)
{
	if (!rendererActive)
	{
		if (compileShaderFromFile(
			"resources/shaders/base.vert",
			SHADER_VERTEX,
			&vertexShader) == -1)
		{
			LOG("Unable to compile vertex shader from file\n");
		}

		if (compileShaderFromFile(
			"resources/shaders/color.frag",
			SHADER_FRAGMENT,
			&fragmentShader) == -1)
		{
			LOG("Unable to compile fragment shader from file\n");
		}

		Shader *program[2];
		program[0] = &vertexShader;
		program[1] = &fragmentShader;

		if (composeShaderPipeline(program, 2, &pipeline) == -1)
		{
			LOG("Unable to compose shader program\n");
		}

		freeShader(vertexShader);
		freeShader(fragmentShader);
		free(pipeline.shaders);
		pipeline.shaderCount = 0;

		getUniform(pipeline, "model", UNIFORM_MAT4, &modelUniform);
		getUniform(pipeline, "view", UNIFORM_MAT4, &viewUniform);
		getUniform(pipeline, "projection", UNIFORM_MAT4, &projectionUniform);

		getUniform(pipeline, "material", UNIFORM_TEXTURE_2D, &materialUniform);
		getUniform(
			pipeline,
			"materialValues",
			UNIFORM_VEC3,
			&materialValuesUniform);
		getUniform(
			pipeline,
			"materialMask",
			UNIFORM_TEXTURE_2D,
			&materialMaskUniform);
		getUniform(
			pipeline,
			"opacityMask",
			UNIFORM_TEXTURE_2D,
			&opacityMaskUniform);
		getUniform(
			pipeline,
			"collectionMaterial",
			UNIFORM_TEXTURE_2D,
			&collectionMaterialUniform);
		getUniform(
			pipeline,
			"collectionMaterialValues",
			UNIFORM_VEC3,
			&collectionMaterialValuesUniform);
		getUniform(
			pipeline,
			"grungeMaterial",
			UNIFORM_TEXTURE_2D,
			&grungeMaterialUniform);
		getUniform(
			pipeline,
			"grungeMaterialValues",
			UNIFORM_VEC3,
			&grungeMaterialValuesUniform);
		getUniform(
			pipeline,
			"wearMaterial",
			UNIFORM_TEXTURE_2D,
			&wearMaterialUniform);
		getUniform(
			pipeline,
			"wearMaterialValues",
			UNIFORM_VEC3,
			&wearMaterialValuesUniform);

		getUniform(
			pipeline,
			"useCustomColor",
			UNIFORM_BOOL,
			&useCustomColorUniform);
		getUniform(
			pipeline,
			"customColor",
			UNIFORM_VEC3,
			&customColorUniform);

		rendererActive = true;
	}
}

extern real64 alpha;

internal
void beginRendererSystem(Scene *scene, real64 dt)
{
	bindShaderPipeline(pipeline);

	if (cameraSetUniforms(
		scene,
		viewUniform,
		projectionUniform,
		pipeline) == -1)
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

			GLenum glError = glGetError();
			if (glError != GL_NO_ERROR)
			{
				LOG("Error when drawing model (%s), subset (%s): %s\n",
					model->name.string,
					subset->name.string,
					gluErrorString(glError));
			}
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

			GLenum glError = glGetError();
			if (glError != GL_NO_ERROR)
			{
				LOG("Error when drawing wireframe for model (%s), "
					"subset (%s): %s\n",
					model->name.string,
					subset->name.string,
					gluErrorString(glError));
			}

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
	unbindShaderPipeline();
}

internal
void shutdownRendererSystem(Scene *scene)
{
	rendererActive = false;
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

int32 setMaterialUniform(Uniform *uniform, GLint *textureIndex)
{
	GLint materialTextureIndices[MATERIAL_COMPONENT_TYPE_COUNT];
	for (uint8 i = 0; i < MATERIAL_COMPONENT_TYPE_COUNT; i++)
	{
		materialTextureIndices[i] = (*textureIndex)++;
	}

	if (setUniform(
		*uniform,
		MATERIAL_COMPONENT_TYPE_COUNT,
		materialTextureIndices) == -1)
	{
		return -1;
	}

	return 0;
}

int32 setMaterialValuesUniform(Uniform *uniform, Material *material)
{
	kmVec3 materialValues[MATERIAL_COMPONENT_TYPE_COUNT];
	for (uint8 i = 0; i < MATERIAL_COMPONENT_TYPE_COUNT; i++)
	{
		materialValues[i] = material->components[i].value;
	}

	if (setUniform(
		*uniform,
		MATERIAL_COMPONENT_TYPE_COUNT,
		materialValues) == -1)
	{
		return -1;
	}

	return 0;
}

void activateMaterialTextures(Material *material, GLint *textureIndex)
{
	for (uint8 i = 0; i < MATERIAL_COMPONENT_TYPE_COUNT; i++)
	{
		activateTexture(material->components[i].texture, textureIndex);
	}
}

void activateTexture(UUID name, GLint *textureIndex)
{
	Texture *texture = getTexture(name.string);
	if (texture)
	{
		glActiveTexture(GL_TEXTURE0 + *textureIndex);
		glBindTexture(GL_TEXTURE_2D, texture->id);
	}

	(*textureIndex)++;
}