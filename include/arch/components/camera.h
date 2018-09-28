#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

#include "renderer/renderer_types.h"

void cameraSetUniforms(
	CameraComponent *camera,
	TransformComponent *transform,
	Uniform viewUniform,
	Uniform projectionUniform);