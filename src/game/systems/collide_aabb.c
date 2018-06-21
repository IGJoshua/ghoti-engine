#include "defines.h"

#include "data/data_types.h"
#include "data/list.h"
#include "data/hash_map.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"

#include "components/component_types.h"

#include <kazmath/aabb3.h>

#include <string.h>
#include <stdio.h>

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

	kmAABB3 AABB;
	AABBComponent *thisAABBComponent = sceneGetComponentFromEntity(
		scene,
		entityID,
		aabbComponentID);
	kmAABB3Initialize(
		&AABB,
		&transform->globalPosition,
		thisAABBComponent->bounds.x * transform->globalScale.x * 2,
		thisAABBComponent->bounds.y * transform->globalScale.y * 2,
		thisAABBComponent->bounds.z * transform->globalScale.z * 2);

	printf("Testing entity %s\n", entityID.string);
	kmVec3 center = {};
	kmAABB3Centre(&AABB, &center);
	kmVec3 extents = {};
	extents.x = kmAABB3DiameterX(&AABB);
	extents.y = kmAABB3DiameterY(&AABB);
	extents.z = kmAABB3DiameterZ(&AABB);
	printf("Center %f, %f, %f\n", center.x, center.y, center.z);
	printf("Extents %f, %f, %f\n", extents.x / 2, extents.y / 2, extents.z / 2);

	kmAABB3 otherAABB;
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
			kmAABB3Initialize(
				&otherAABB,
				&otherTransform->globalPosition,
				otherAABBComponent->bounds.x * otherTransform->globalScale.x * 2,
				otherAABBComponent->bounds.y * otherTransform->globalScale.y * 2,
				otherAABBComponent->bounds.z * otherTransform->globalScale.z * 2);

			printf("Other AABB %s\n", cdtIteratorGetUUID(itr)->string);
			kmAABB3Centre(&otherAABB, &center);
			printf("Center %f, %f, %f\n", center.x, center.y, center.z);
			extents.x = kmAABB3DiameterX(&AABB);
			extents.y = kmAABB3DiameterY(&AABB);
			extents.z = kmAABB3DiameterZ(&AABB);
			printf("Extents %f, %f, %f\n", extents.x / 2, extents.y / 2, extents.z / 2);

			// On a collision, create a new hit entity and push it to the front of the list
			if (kmAABB3IntersectsAABB(&AABB, &otherAABB))
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
