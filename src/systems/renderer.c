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

Shader vertShader;
Shader fragShader;

ShaderPipeline pipeline;

Uniform modelUniform;
Uniform viewUniform;
Uniform projectionUniform;

Uniform diffuseTextureUniform;

System createRendererSystem()
{
	System renderer = {};

	UUID transformComponentID = {};
	strcpy(transformComponentID.string, "transform");
	UUID modelComponentID = {};
	strcpy(modelComponentID.string, "model");

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
	vertShader = compileShaderFromFile("resources/shaders/base.vert", SHADER_VERTEX);
	fragShader = compileShaderFromFile("resources/shaders/color.frag", SHADER_FRAGMENT);

	{
		Shader *program[2];
		program[0] = &vertShader;
		program[1] = &fragShader;

		pipeline = composeShaderPipeline(program, 2);
	}

	freeShader(vertShader);
	freeShader(fragShader);
	free(pipeline.shaders);
	pipeline.shaderCount = 0;

	modelUniform = getUniform(pipeline, "model", UNIFORM_MAT4);
	viewUniform = getUniform(pipeline, "view", UNIFORM_MAT4);
	projectionUniform = getUniform(pipeline, "projection", UNIFORM_MAT4);

	diffuseTextureUniform = getUniform(pipeline, "diffuseTexture", UNIFORM_TEXTURE_2D);
}

void runRendererSystem(Scene *scene, UUID entityID)
{
	UUID modelComponentID = {};
	strcpy(modelComponentID.string, "model");
	ModelComponent *model = sceneGetComponentFromEntity(scene, entityID, modelComponentID);

	if (!getModel(model->name))
	{
		loadModel(model->name);
	}

	UUID transformComponentID = {};
	strcpy(transformComponentID.string, "transform");
	TransformComponent *transform = sceneGetComponentFromEntity(scene, entityID, transformComponentID);

	kmMat3 rotation;
	kmMat3FromRotationQuaternion(&rotation, &transform->rotation);

	kmMat4 worldMatrix;
	kmMat4RotationTranslation(&worldMatrix, &rotation, &transform->position);
	kmMat4 scalingMatrix;
	kmMat4Scaling(&scalingMatrix, transform->scale.x, transform->scale.y, transform->scale.z);
	kmMat4Multiply(&worldMatrix, &worldMatrix, &scalingMatrix);

	kmMat4 view;
	kmMat4Translation(&view, 0, 0, 2);
	kmMat4Inverse(&view, &view);
	kmMat4 projection;
	kmMat4PerspectiveProjection(&projection, 90, 16.0f / 9.0f, 0.1f, 1000.0f);

	renderModel(model->name, &worldMatrix, &view, &projection);
}

void shutdownRendererSystem(Scene *scene)
{

}

inline
void freeRendererSystem(System *renderer)
{
	listClear(&renderer->componentTypes);
}
