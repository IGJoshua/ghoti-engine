#include "systems.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/system.h"

#include <string.h>

#define SYSTEM(fn) System create ## fn ## System(void)
#define REGISTER_SYSTEM(sys, name) System sys = create ## sys ## System();\
	key = idFromName(name);\
	hashMapInsert(systemRegistry, &key, &sys);

SYSTEM(Renderer);
SYSTEM(CleanGlobalTransforms);
SYSTEM(ApplyParentTransforms);
SYSTEM(RenderBox);
SYSTEM(CleanHitInformation);
SYSTEM(SimulateRigidbodies);
SYSTEM(CleanHitList);
SYSTEM(RenderHeightmap);
SYSTEM(JointInformation);
SYSTEM(GUI);

extern HashMap systemRegistry;

void initSystems(void)
{
	systemRegistry = createHashMap(
		sizeof(UUID),
		sizeof(System),
		SYSTEM_BUCKET_COUNT,
		(ComparisonOp)&strcmp);

	UUID key;

	REGISTER_SYSTEM(Renderer, "renderer");
	REGISTER_SYSTEM(CleanGlobalTransforms, "clean_global_transforms");
	REGISTER_SYSTEM(ApplyParentTransforms, "apply_parent_transforms");
	REGISTER_SYSTEM(RenderBox, "render_box");
	REGISTER_SYSTEM(CleanHitInformation, "clean_hit_information");
	REGISTER_SYSTEM(SimulateRigidbodies, "simulate_rigid_bodies");
	REGISTER_SYSTEM(CleanHitList, "clean_hit_list");
	REGISTER_SYSTEM(RenderHeightmap, "render_heightmap");
	REGISTER_SYSTEM(JointInformation, "joint_information");
	REGISTER_SYSTEM(GUI, "gui");
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
