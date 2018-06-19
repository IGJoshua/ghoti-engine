#include "systems.h"

#include "asset_management/model.h"

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
			return;
		}

		if (compileShaderFromFile(
			"resources/shaders/color.frag",
			SHADER_FRAGMENT,
			&fragShader) == -1)
		{
			return;
		}

		Shader *program[2];
		program[0] = &vertShader;
		program[1] = &fragShader;

		if (composeShaderPipeline(program, 2, &pipeline) == -1)
		{
			return;
		}

		freeShader(vertShader);
		freeShader(fragShader);
		free(pipeline.shaders);
		pipeline.shaderCount = 0;

		if (getUniform(pipeline, "model", UNIFORM_MAT4, &modelUniform) == -1)
		{
			return;
		}

		if (getUniform(pipeline, "view", UNIFORM_MAT4, &viewUniform) == -1)
		{
			return;
		}

		if (getUniform(
			pipeline,
			"projection",
			UNIFORM_MAT4,
			&projectionUniform) == -1)
		{
			return;
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
			return;
		}

		if (getUniform(
			pipeline,
			"specularTexture",
			UNIFORM_TEXTURE_2D,
			&textureUniforms[MATERIAL_COMPONENT_TYPE_SPECULAR]) == -1)
		{
			return;
		}

		if (getUniform(
			pipeline,
			"normalTexture",
			UNIFORM_TEXTURE_2D,
			&textureUniforms[MATERIAL_COMPONENT_TYPE_NORMAL]) == -1)
		{
			return;
		}

		if (getUniform(
			pipeline,
			"emissiveTexture",
			UNIFORM_TEXTURE_2D,
			&textureUniforms[MATERIAL_COMPONENT_TYPE_EMISSIVE]) == -1)
		{
			return;
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
}

internal
void runRendererSystem(Scene *scene, UUID entityID, real64 dt)
{
	ModelComponent *model = sceneGetComponentFromEntity(
		scene,
		entityID,
		modelComponentID);

	if (!getModel(model->name))
	{
		if (loadModel(model->name) == -1)
		{
			return;
		}
	}

	if (!model->visible)
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

	if (renderModel(model->name, &worldMatrix, &view, &projection) == -1)
	{
		return;
	}
}

internal
void endRendererSystem(Scene *scene, real64 dt)
{

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
