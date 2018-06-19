#include "systems.h"

#include "asset_management/model.h"
#include "asset_management/texture.h"

#include "renderer/renderer_types.h"
#include "renderer/shader.h"

#include "ECS/scene.h"

#include "components/component_types.h"

#include "data/list.h"

#include <kazmath/mat4.h>
#include <kazmath/mat3.h>
#include <kazmath/quaternion.h>

#include <string.h>
#include <malloc.h>

extern Shader vertShader;
extern Shader fragShader;

extern ShaderPipeline pipeline;

extern Uniform modelUniform;
extern Uniform viewUniform;
extern Uniform projectionUniform;

extern Uniform textureUniforms[MATERIAL_COMPONENT_TYPE_COUNT];

extern bool rendererActive;

internal UUID transformComponentID = {};
internal UUID modelComponentID = {};
internal UUID cameraComponentID = {};

internal
void initRendererSystem(Scene *scene)
{
	if (!rendererActive)
	{
		if (compileShaderFromFile(
			"resources/shaders/base.vert",
			SHADER_VERTEX,
			&vertShader) == -1)
		{
			printf("Unable to compile vertex shader from file\n");
		}

		if (compileShaderFromFile(
			"resources/shaders/color.frag",
			SHADER_FRAGMENT,
			&fragShader) == -1)
		{
			printf("Unable to compile fragment shader from file\n");
		}

		Shader *program[2];
		program[0] = &vertShader;
		program[1] = &fragShader;

		if (composeShaderPipeline(program, 2, &pipeline) == -1)
		{
			printf("Unable to compose shader program\n");
		}

		freeShader(vertShader);
		freeShader(fragShader);
		free(pipeline.shaders);
		pipeline.shaderCount = 0;

		if (getUniform(pipeline, "model", UNIFORM_MAT4, &modelUniform) == -1)
		{
			printf("Unable to get model component uniform\n");
		}

		if (getUniform(pipeline, "view", UNIFORM_MAT4, &viewUniform) == -1)
		{
			printf("Unable to get view component uniform\n");
		}

		if (getUniform(
			pipeline,
			"projection",
			UNIFORM_MAT4,
			&projectionUniform) == -1)
		{
			printf("Unable to get projection component uniform\n");
		}

		for (uint8 i = 0; i < MATERIAL_COMPONENT_TYPE_COUNT; i++)
		{
			textureUniforms[i].type = UNIFORM_INVALID;
		}

		if (getUniform(
			pipeline,
			"diffuseTexture",
			UNIFORM_TEXTURE_2D,
			&textureUniforms[MATERIAL_COMPONENT_TYPE_DIFFUSE]) == -1)
		{
			printf("Unable to get diffuse texture uniform\n");
		}

		if (getUniform(
			pipeline,
			"specularTexture",
			UNIFORM_TEXTURE_2D,
			&textureUniforms[MATERIAL_COMPONENT_TYPE_SPECULAR]) == -1)
		{
			printf("Unable to get specular texture uniform\n");
		}

		if (getUniform(
			pipeline,
			"normalTexture",
			UNIFORM_TEXTURE_2D,
			&textureUniforms[MATERIAL_COMPONENT_TYPE_NORMAL]) == -1)
		{
			printf("Unable to get normal texture uniform\n");
		}

		if (getUniform(
			pipeline,
			"emissiveTexture",
			UNIFORM_TEXTURE_2D,
			&textureUniforms[MATERIAL_COMPONENT_TYPE_EMISSIVE]) == -1)
		{
			printf("Unable to get emissive texture uniform\n");
		}

		rendererActive = true;
	}
}

internal CameraComponent *camera = 0;
internal TransformComponent *cameraTransform = 0;
internal kmMat4 view = {};
internal kmMat4 projection = {};

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

	kmMat3 cameraRotation;
	kmMat3FromRotationQuaternion(&cameraRotation, &cameraTransform->rotation);

	kmMat4RotationTranslation(&view, &cameraRotation, &cameraTransform->position);
	kmMat4Inverse(&view, &view);

	kmMat4PerspectiveProjection(
		&projection,
		camera->fov,
		camera->aspectRatio,
		camera->nearPlane,
		camera->farPlane);

	bindShaderPipeline(pipeline);

	if (setUniform(viewUniform, &view) == -1)
	{
		printf("Unable to set view uniform\n");
	}

	if (setUniform(projectionUniform, &projection) == -1)
	{
		printf("Unable to set projection uniform\n");
	}

	for (GLint i = 0; i < MATERIAL_COMPONENT_TYPE_COUNT; i++)
	{
		if (textureUniforms[i].type != UNIFORM_INVALID)
		{
			if (setUniform(textureUniforms[i], &i) == -1)
			{
				printf("Unable to set texture uniform %d\n", i);
			}
		}
	}
}

extern real64 alpha;

internal
void runRendererSystem(Scene *scene, UUID entityID, real64 dt)
{
	ModelComponent *modelComponent = sceneGetComponentFromEntity(
		scene,
		entityID,
		modelComponentID);

	Model *model = getModel(modelComponent->name);
	if (!model)
	{
		if (loadModel(modelComponent->name) == -1)
		{
			return;
		}
	}

	if (!modelComponent->visible)
	{
		return;
	}

	TransformComponent *transform = sceneGetComponentFromEntity(
		scene,
		entityID,
		transformComponentID);

	kmMat3 rotation;
	kmMat3FromRotationQuaternion(&rotation, &transform->rotation);

	kmMat4 worldMatrix;
	kmMat4RotationTranslation(&worldMatrix, &rotation, &transform->position);
	kmMat4 scalingMatrix;
	kmMat4Scaling(
		&scalingMatrix,
		transform->scale.x,
		transform->scale.y,
		transform->scale.z);
	kmMat4Multiply(&worldMatrix, &worldMatrix, &scalingMatrix);

	if (setUniform(modelUniform, &worldMatrix) == -1)
	{
		printf("Unable to set model uniform\n");
		return;
	}

	for (uint32 i = 0; i < model->numSubsets; i++)
	{
		Mesh *mesh = &model->meshes[i];

		glBindVertexArray(mesh->vertexArray);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);

		for (uint8 j = 0; j < NUM_VERTEX_ATTRIBUTES; j++)
		{
			glEnableVertexAttribArray(j);
		}

		Material *material = &model->materials[i];

		Texture *textures[MATERIAL_COMPONENT_TYPE_COUNT];
		memset(textures, 0, sizeof(Texture*) * MATERIAL_COMPONENT_TYPE_COUNT);

		for (uint8 j = 0; j < MATERIAL_COMPONENT_TYPE_COUNT; j++)
		{
			textures[j] = getTexture(material->components[j].texture);

			if (textures[j])
			{
				glActiveTexture(GL_TEXTURE0 + j);
				glBindTexture(GL_TEXTURE_2D, textures[j]->id);
			}
		}

		glDrawElements(
			GL_TRIANGLES,
			mesh->numIndices,
			GL_UNSIGNED_INT,
			NULL);
		GLenum glError = glGetError();
		if (glError != GL_NO_ERROR)
		{
			printf(
				"Error in Draw Subset %s in Model (%s): %s\n",
				material->name,
				modelComponent->name,
				gluErrorString(glError));
		}
	}
}

internal
void endRendererSystem(Scene *scene, real64 dt)
{
	for (uint8 j = 0; j < MATERIAL_COMPONENT_TYPE_COUNT; j++)
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
	glUseProgram(0);
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
