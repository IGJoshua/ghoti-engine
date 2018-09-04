#include "components/camera.h"

#include "components/component_types.h"
#include "components/transform.h"

#include "core/log.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "renderer/shader.h"

#include <kazmath/mat4.h>

extern real64 alpha;

int32 cameraSetUniforms(
	Scene *scene,
	Uniform viewUniform,
	Uniform projectionUniform)
{
	CameraComponent *camera = sceneGetComponentFromEntity(
		scene,
		scene->mainCamera,
		idFromName("camera"));

	if (!camera)
	{
		return -1;
	}

	TransformComponent *cameraTransform = sceneGetComponentFromEntity(
		scene,
		scene->mainCamera,
		idFromName("transform"));

	kmMat4 view = tGetInterpolatedTransformMatrix(cameraTransform, alpha);
	kmMat4Inverse(&view, &view);

	kmMat4 projection = {};

	kmMat4PerspectiveProjection(
		&projection,
		camera->fov,
		camera->aspectRatio,
		camera->nearPlane,
		camera->farPlane);

	setUniform(viewUniform, 1, &view);
	setUniform(projectionUniform, 1, &projection);

	return 0;
}
