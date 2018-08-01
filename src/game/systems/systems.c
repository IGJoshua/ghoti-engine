#include "systems.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/system.h"

#include <string.h>

#define SYSTEM(fn) System fn(void)
#define REGISTER_SYSTEM(sys, fn, name) System sys = fn();\
	key = idFromName(name);\
	hashMapInsert(systemRegistry, &key, &sys);

SYSTEM(createRendererSystem);
SYSTEM(createCleanGlobalTransformsSystem);
SYSTEM(createApplyParentTransformsSystem);
SYSTEM(createRenderBoxSystem);
SYSTEM(createCleanHitInformationSystem);
SYSTEM(createSimulateRigidbodiesSystem);
SYSTEM(createCleanHitListSystem);
SYSTEM(createJointInformationSystem);

extern HashMap systemRegistry;

void initSystems(void)
{
	systemRegistry = createHashMap(
		sizeof(UUID),
		sizeof(System),
		SYSTEM_BUCKET_COUNT,
		(ComparisonOp)&strcmp);

	UUID key;

	REGISTER_SYSTEM(
		renderer,
		createRendererSystem,
		"renderer");

	REGISTER_SYSTEM(
		cleanGlobalTransforms,
		createCleanGlobalTransformsSystem,
		"clean_global_transforms");

	REGISTER_SYSTEM(
		applyParentTransforms,
		createApplyParentTransformsSystem,
		"apply_parent_transforms");

	REGISTER_SYSTEM(
		renderBox,
		createRenderBoxSystem,
		"render_box");

	REGISTER_SYSTEM(
		cleanHitInformation,
		createCleanHitInformationSystem,
		"clean_hit_information");

	REGISTER_SYSTEM(
		simulateRigidbodies,
		createSimulateRigidbodiesSystem,
		"simulate_rigid_bodies");

	REGISTER_SYSTEM(
		cleanHitList,
		createCleanHitListSystem,
		"clean_hit_list");

	REGISTER_SYSTEM(
		jointInformation,
		createJointInformationSystem,
		"joint_information");
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
