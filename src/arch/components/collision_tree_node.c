#include "components/collision_tree_node.h"

#include "data/hash_map.h"

#include "ECS/scene.h"
#include "ECS/component.h"

#include <ode/ode.h>

void removeCollisionTreeNode(
	Scene *scene,
	UUID entity,
	CollisionTreeNodeComponent *node)
{
	UUID collisionComponentID = idFromName("collision");
	UUID nodeComponentID = idFromName("collision_tree_node");
	UUID emptyID = idFromName("");

	CollisionComponent *collisionComponent = sceneGetComponentFromEntity(
		scene,
		node->collisionVolume,
		collisionComponentID);

	UUID nodeID = collisionComponent->collisionTree;

	CollisionTreeNodeComponent *nodeComponent = sceneGetComponentFromEntity(
		scene,
		nodeID,
		nodeComponentID);

	if (!strcmp(entity.string, nodeID.string))
	{
		collisionComponent->collisionTree = emptyID;
		if (nodeComponent)
		{
			collisionComponent->collisionTree = nodeComponent->nextCollider;
		}
	}
	else
	{
		while (nodeComponent)
		{
			CollisionTreeNodeComponent *previousNodeComponent = nodeComponent;

			nodeID = nodeComponent->nextCollider;
			nodeComponent = sceneGetComponentFromEntity(
				scene,
				nodeID,
				nodeComponentID);

			if (!strcmp(
				entity.string,
				previousNodeComponent->nextCollider.string))
			{
				previousNodeComponent->nextCollider = emptyID;
				if (nodeComponent)
				{
					previousNodeComponent->nextCollider =
						nodeComponent->nextCollider;
				}

				break;
			}
		}
	}

	free(dGeomGetData(node->geomID));
	dGeomSetData(node->geomID, 0);
	dGeomDestroy(node->geomID);
}