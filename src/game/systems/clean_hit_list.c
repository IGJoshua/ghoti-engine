#include "defines.h"

#include "data/data_types.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"

#include "components/component_types.h"

internal UUID collisionComponentID = {};
internal UUID hitListComponentID = {};

internal
void runCleanHitListSystem(Scene *scene, UUID entityID, real64 dt)
{
	CollisionComponent *collision = sceneGetComponentFromEntity(
		scene,
		entityID,
		collisionComponentID);

	UUID nextItem = {};
	for (UUID currentListItem = collision->lastHitList;
		 strcmp(currentListItem.string, "");
		 currentListItem = nextItem)
	{
		HitListComponent *hitList = sceneGetComponentFromEntity(
			scene,
			currentListItem,
			hitListComponentID);

		nextItem = hitList->nextHit;

		sceneRemoveEntity(scene, currentListItem);
	}
}

System createCleanHitListSystem(void)
{
	collisionComponentID = idFromName("collision");
	hitListComponentID = idFromName("hit_list");

	System ret = {};

	ret.componentTypes = createList(sizeof(UUID));
	listPushFront(&ret.componentTypes, &collisionComponentID);

	ret.run = &runCleanHitListSystem;

	return ret;
}
