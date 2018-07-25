#include "defines.h"

#include "data/data_types.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "components/component_types.h"

#include <string.h>

internal UUID hitInformationComponentID = {};

internal
void runCleanHitInformationSystem(Scene *scene, UUID entityID, real64 dt)
{
	HitInformationComponent *hitInformation = sceneGetComponentFromEntity(
		scene,
		entityID,
		hitInformationComponentID);

	if (hitInformation->age >= 0)
	{
		if (++hitInformation->age > 2)
		{
			sceneRemoveEntity(scene, entityID);
		}
	}
}

System createCleanHitInformationSystem(void)
{
	hitInformationComponentID = idFromName("hit_information");

	System ret = {};

	ret.componentTypes = createList(sizeof(UUID));
	listPushFront(&ret.componentTypes, &hitInformationComponentID);

	ret.init = 0;
	ret.begin = 0;
	ret.run = &runCleanHitInformationSystem;
	ret.end = 0;
	ret.shutdown = 0;

	return ret;
}
