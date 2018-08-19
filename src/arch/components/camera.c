#include "components/camera.h"

#include "components/component_types.h"

#include "core/log.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "renderer/shader.h"

#include <kazmath/mat3.h>
#include <kazmath/mat4.h>

extern real64 alpha;

int32 cameraSetUniforms(
	Scene *scene,
	Uniform viewUniform,
	Uniform projectionUniform,
	ShaderPipeline pipeline)
{
	CameraComponent *camera = 0;
	TransformComponent *cameraTransform = 0;

	camera = sceneGetComponentFromEntity(
		scene,
		scene->mainCamera,
		idFromName("camera"));
	cameraTransform = sceneGetComponentFromEntity(
		scene,
		scene->mainCamera,
		idFromName("transform"));

	kmQuaternion cameraRotationQuat;
	kmQuaternionSlerp(
		&cameraRotationQuat,
		&cameraTransform->lastGlobalRotation,
		&cameraTransform->globalRotation,
		alpha);

	kmMat3 cameraRotation;
	kmMat3FromRotationQuaternion(&cameraRotation, &cameraRotationQuat);

	kmVec3 cameraPosition;
	kmVec3Lerp(
		&cameraPosition,
		&cameraTransform->lastGlobalPosition,
		&cameraTransform->globalPosition,
		alpha);

	kmMat4 view = {};

	kmMat4RotationTranslation(
		&view,
		&cameraRotation,
		&cameraPosition);
	kmMat4Inverse(&view, &view);

	kmMat4 projection = {};

	kmMat4PerspectiveProjection(
		&projection,
		camera->fov,
		camera->aspectRatio,
		camera->nearPlane,
		camera->farPlane);

	bindShaderPipeline(pipeline);

	if (setUniform(viewUniform, 1, &view) == -1)
	{
		LOG("Unable to set view uniform\n");
	}

	if (setUniform(projectionUniform, 1, &projection) == -1)
	{
		LOG("Unable to set projection uniform\n");
	}

	return 0;
}
