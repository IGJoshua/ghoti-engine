#include "defines.h"

#include "data/data_types.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "components/component_types.h"

#include <string.h>

internal UUID hitInformationComponentID = {};
internal UUID collisionComponentID = {};
internal UUID emptyID = {};

internal
void runCleanHitInformationSystem(Scene *scene, UUID entityID, real64 dt)
{
	CollisionComponent *collision = sceneGetComponentFromEntity(
		scene,
		entityID,
		collisionComponentID);

	UUID nextHit = {};
	// Iterate over every hit in the last hit list and delete them
	for (UUID itr = collision->lastHitList;
		 strcmp(itr.string, emptyID.string);
		 itr = nextHit)
	{
		nextHit = ((HitInformationComponent *)sceneGetComponentFromEntity(
					   scene,
					   itr,
					   hitInformationComponentID))->nextHit;
		sceneRemoveEntity(scene, itr);
	}

	// Move current hits to last hit list
	collision->lastHitList = collision->hitList;
	collision->hitList = emptyID;
}

System createCleanHitInformationSystem(void)
{
	hitInformationComponentID = idFromName("hit_information");
	collisionComponentID = idFromName("collision");

	System ret = {};

	ret.componentTypes = createList(sizeof(UUID));
	listPushFront(&ret.componentTypes, &collisionComponentID);

	ret.init = 0;
	ret.begin = 0;
	ret.run = &runCleanHitInformationSystem;
	ret.end = 0;
	ret.shutdown = 0;

	return ret;
}
