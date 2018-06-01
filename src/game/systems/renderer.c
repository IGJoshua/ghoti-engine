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

extern Uniform diffuseTextureUniform;

System createRendererSystem()
{
	System renderer = {};

	UUID transformComponentID = idFromName("transform");
	UUID modelComponentID = idFromName("model");

	List componentList = createList(sizeof(UUID));

	listPushFront(&componentList, &transformComponentID);
	listPushFront(&componentList, &modelComponentID);

	renderer.componentTypes = componentList;

	renderer.init = &initRendererSystem;
	renderer.fn = &runRendererSystem;
	renderer.shutdown = &shutdownRendererSystem;

	return renderer;
}

void initRendererSystem(Scene *scene)
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

	if (getUniform(pipeline, "projection", UNIFORM_MAT4, &projectionUniform) == -1)
	{
		return;
	}

	if (getUniform(
		pipeline,
		"diffuseTexture",
		UNIFORM_TEXTURE_2D,
		&diffuseTextureUniform) == -1)
	{
		return;
	}
}

void runRendererSystem(Scene *scene, UUID entityID, real64 dt)
{
	UUID modelComponentID = idFromName("model");
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

	UUID transformComponentID = idFromName("transform");
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

	UUID cameraComponentID = idFromName("camera");
	CameraComponent *camera =
		sceneGetComponentFromEntity(
			scene,
			scene->mainCamera,
			cameraComponentID);
	TransformComponent *cameraTransform =
		sceneGetComponentFromEntity(
			scene,
			scene->mainCamera,
			transformComponentID);

	kmMat3 cameraRotation;
	kmMat3FromRotationQuaternion(&cameraRotation, &cameraTransform->rotation);

	kmMat4 view;
	kmMat4RotationTranslation(&view, &cameraRotation, &cameraTransform->position);
	kmMat4Inverse(&view, &view);

	kmMat4 projection;
	kmMat4PerspectiveProjection(
		&projection,
		camera->fov,
		camera->aspectRatio,
		camera->nearPlane,
		camera->farPlane);

	if (renderModel(model->name, &worldMatrix, &view, &projection) == -1)
	{
		return;
	}
}

void shutdownRendererSystem(Scene *scene)
{

}

inline
void freeRendererSystem(System *renderer)
{
	listClear(&renderer->componentTypes);
}
