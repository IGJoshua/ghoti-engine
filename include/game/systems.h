#pragma once
#include "defines.h"

#include "ECS/ecs_types.h"

System createRendererSystem();
void initRendererSystem(Scene *scene);
void runRendererSystem(Scene *scene, UUID entityID, real64 dt);
void shutdownRendererSystem(Scene *scene);
void freeRendererSystem(System *renderer);
