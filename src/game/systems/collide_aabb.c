#include "defines.h"

#include "data/data_types.h"
#include "data/list.h"
#include "data/hash_map.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"

#include "components/component_types.h"

#include <string.h>
#include <stdio.h>
#include <math.h>

internal UUID transformComponentID = {};
internal UUID aabbComponentID = {};
internal UUID collisionTreeNodeComponentID = {};
internal UUID collisionComponentID = {};

void runCollideAABBSystem(Scene *scene, UUID entityID, real64 dt)
{
	TransformComponent *transform = sceneGetComponentFromEntity(
		scene,
		entityID,
		transformComponentID);

	CollisionComponent *collision = sceneGetComponentFromEntity(
		scene,
		entityID,
		collisionComponentID);

	AABBComponent *thisAABBComponent = sceneGetComponentFromEntity(
		scene,
		entityID,
		aabbComponentID);

	TransformComponent *otherTransform;
	AABBComponent *otherAABBComponent;
	// Loop over every AABB in the scene
	for (ComponentDataTableIterator itr = cdtGetIterator(
				 *(ComponentDataTable **)hashMapGetKey(
					 scene->componentTypes,
					 &aabbComponentID));
		 !cdtIteratorAtEnd(itr);
		 cdtMoveIterator(&itr))
	{
		if (!strcmp(entityID.string, cdtIteratorGetUUID(itr)->string))
		{
			continue;
		}

		otherTransform = sceneGetComponentFromEntity(
				scene,
				*cdtIteratorGetUUID(itr),
				transformComponentID);

		if (otherTransform)
		{
			// Check for collision
			otherAABBComponent = cdtIteratorGetData(itr);

			// On a collision, create a new hit entity and push it to the front of the list
			bool collided = 1;

			if (fabsf(transform->globalPosition.x - otherTransform->globalPosition.x)
				> thisAABBComponent->bounds.x * transform->globalScale.x
				+ otherAABBComponent->bounds.x * otherTransform->globalScale.x)
			{
				collided = 0;
			}
			if (fabsf(transform->globalPosition.y - otherTransform->globalPosition.y)
				> thisAABBComponent->bounds.y * transform->globalScale.y
				+ otherAABBComponent->bounds.y * otherTransform->globalScale.y)
			{
				collided = 0;
			}
			if (fabsf(transform->globalPosition.z - otherTransform->globalPosition.z)
				> thisAABBComponent->bounds.z * transform->globalScale.z
				+ otherAABBComponent->bounds.z * otherTransform->globalScale.z)
			{
				collided = 0;
			}

			if (collided)
			{
				UUID hitInformation = sceneCreateEntity(scene);
				HitInformationComponent hitInformationComponent = {};
				hitInformationComponent.otherObject = *cdtIteratorGetUUID(itr);
				hitInformationComponent.nextHit = collision->hitList;
				collision->hitList = hitInformation;
				sceneAddComponentToEntity(scene, hitInformation, idFromName("hit_information"), &hitInformationComponent);
			}
		}
	}
}

System createCollideAABBSystem(void)
{
	transformComponentID = idFromName("transform");
	aabbComponentID = idFromName("aabb");
	collisionComponentID = idFromName("collision");
	collisionTreeNodeComponentID = idFromName("collision_tree_node");

	System ret = {};

	ret.componentTypes = createList(sizeof(UUID));
	listPushFront(&ret.componentTypes, &transformComponentID);
	listPushFront(&ret.componentTypes, &aabbComponentID);
	listPushFront(&ret.componentTypes, &collisionComponentID);

	ret.init = 0;
	ret.begin = 0;
	ret.run = &runCollideAABBSystem;
	ret.end = 0;
	ret.shutdown = 0;

	return ret;
}
