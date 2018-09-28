#include "components/component_types.h"
#include "components/camera.h"
#include "components/transform.h"

#include "core/log.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "renderer/shader.h"

#include <kazmath/mat4.h>

extern real64 alpha;

void cameraSetUniforms(
	CameraComponent *camera,
	TransformComponent *transform,
	Uniform viewUniform,
	Uniform projectionUniform)
{
	kmMat4 view = tGetInterpolatedTransformMatrix(transform, alpha);
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
}
