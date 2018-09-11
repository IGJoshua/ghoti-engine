#include "components/animation.h"

#include "data/data_types.h"
#include "data/hash_map.h"

#include "ECS/scene.h"

HashMap skeletonsMap;

internal void loadSkeleton(HashMap skeleton, Scene *scene, UUID joint);

void addSkeleton(Scene *scene, UUID skeletonID)
{
	HashMap *skeletons = hashMapGetData(skeletonsMap, &scene);
	if (!hashMapGetData(*skeletons, &skeletonID))
	{
		HashMap skeleton = createHashMap(
			sizeof(UUID),
			sizeof(JointTransform),
			SKELETON_BUCKET_COUNT,
			(ComparisonOp)&strcmp);
		loadSkeleton(skeleton, scene, skeletonID);
		hashMapInsert(*skeletons, &skeletonID, &skeleton);
	}
}

void loadSkeleton(HashMap skeleton, Scene *scene, UUID joint)
{
	UUID transformComponentID = idFromName("transform");
	UUID jointComponentID = idFromName("joint");

	JointComponent *jointComponent = sceneGetComponentFromEntity(
		scene,
		joint,
		jointComponentID);
	TransformComponent *transform = sceneGetComponentFromEntity(
		scene,
		joint,
		transformComponentID);

	if (!transform)
	{
		return;
	}

	if (jointComponent)
	{
		JointTransform jointTransform;
		jointTransform.uuid = joint;
		jointTransform.transform = transform;

		UUID name = idFromName(jointComponent->name);
		hashMapInsert(skeleton, &name, &jointTransform);
	}

	UUID child = transform->firstChild;

	do
	{
		transform = sceneGetComponentFromEntity(
			scene,
			child,
			transformComponentID);

		if (transform)
		{
			loadSkeleton(skeleton, scene, child);
		}
		else
		{
			break;
		}

		child = transform->nextSibling;
	} while (true);
}

void removeSkeleton(Scene *scene, UUID skeletonID)
{
	HashMap *skeletons = hashMapGetData(skeletonsMap, &scene);
	if (skeletons)
	{
		HashMap *skeleton = hashMapGetData(*skeletons, &skeletonID);
		if (skeleton)
		{
			freeHashMap(skeleton);
			hashMapDelete(*skeletons, &skeletonID);
		}
	}

	sceneRemoveEntity(scene, skeletonID);
}