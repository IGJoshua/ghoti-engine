#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

#include "renderer/renderer_types.h"

int32 cameraSetUniforms(
	Scene *scene,
	Uniform viewUniform,
	Uniform projectionUniform);