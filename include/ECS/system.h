#pragma once
#include "ECS/ecs_types.h"

System createSystem(
	List components,
	InitSystem init,
	SystemFn fn,
	ShutdownSystem shutdown
);

void systemRun(
	Scene *scene,
	System *system
);

void freeSystem(System *system);
