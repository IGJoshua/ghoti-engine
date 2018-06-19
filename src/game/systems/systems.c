#include "systems.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/system.h"

#include <string.h>

System createRendererSystem(void);
System createCleanGlobalTransformsSystem(void);
System createApplyParentTransformsSystem();

extern HashMap systemRegistry;

void initSystems(void)
{
	systemRegistry = createHashMap(
		sizeof(UUID),
		sizeof(System),
		SYSTEM_BUCKET_COUNT,
		(ComparisonOp)&strcmp);

	UUID key;

	System renderer = createRendererSystem();
	key = idFromName("renderer");
	hashMapInsert(systemRegistry, &key, &renderer);

	System cleanGlobalTransforms = createCleanGlobalTransformsSystem();
	key = idFromName("clean_global_transforms");
	hashMapInsert(systemRegistry, &key, &cleanGlobalTransforms);

	System applyParentTransforms = createApplyParentTransformsSystem();
	key = idFromName("apply_parent_transforms");
	hashMapInsert(systemRegistry, &key, &applyParentTransforms);
}

void freeSystems(void)
{
	// NOTE: If any of the systems have
	//       global shutdown functions, call them here

	for (HashMapIterator itr = hashMapGetIterator(systemRegistry);
		 !hashMapIteratorAtEnd(itr);
		 hashMapMoveIterator(&itr))
	{
		freeSystem(hashMapIteratorGetValue(itr));
	}

	freeHashMap(&systemRegistry);
}
