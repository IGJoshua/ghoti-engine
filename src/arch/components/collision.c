#include "components/collision.h"

#include "ECS/scene.h"

void removeCollisionComponent(Scene *scene, CollisionComponent *coll)
{
	if (!coll)
	{
		return;
	}

	CollisionTreeNodeComponent *node = NULL;
	UUID collisionTreeNodeID = idFromName("collision_tree_node");
	UUID currentCollider = coll->collisionTree;
	UUID nextCollider = {};
	while (currentCollider.string[0] != 0)
	{
		node = (CollisionTreeNodeComponent *)sceneGetComponentFromEntity(
			scene,
			currentCollider,
			collisionTreeNodeID);

		if (!node)
		{
			break;
		}

		nextCollider = node->nextCollider;
		sceneRemoveEntity(scene, currentCollider);
		currentCollider = nextCollider;
	}
}