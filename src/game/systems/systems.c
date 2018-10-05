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

SYSTEM(CleanHitInformation);
SYSTEM(CleanHitList);
SYSTEM(Animation);
SYSTEM(CleanGlobalTransforms);
SYSTEM(ApplyParentTransforms);
SYSTEM(SimulateRigidbodies);
SYSTEM(JointInformation);
SYSTEM(ParticleSimulator);
SYSTEM(GUI);
SYSTEM(Audio);
SYSTEM(Lights);

SYSTEM(Shadows);
SYSTEM(CubemapRenderer);
SYSTEM(RenderHeightmap);
SYSTEM(Renderer);
SYSTEM(WireframeRenderer);
SYSTEM(DebugRenderer);
SYSTEM(CollisionPrimitiveRenderer);
SYSTEM(ParticleRenderer);
SYSTEM(GUIRenderer);
SYSTEM(PostProcessing);

extern HashMap systemRegistry;

void initSystems(void)
{
	systemRegistry = createHashMap(
		sizeof(UUID),
		sizeof(System),
		SYSTEM_BUCKET_COUNT,
		(ComparisonOp)&strcmp);

	UUID key;

	REGISTER_SYSTEM(CleanHitInformation, "clean_hit_information");
	REGISTER_SYSTEM(CleanHitList, "clean_hit_list");
	REGISTER_SYSTEM(Animation, "animation");
	REGISTER_SYSTEM(CleanGlobalTransforms, "clean_global_transforms");
	REGISTER_SYSTEM(ApplyParentTransforms, "apply_parent_transforms");
	REGISTER_SYSTEM(SimulateRigidbodies, "simulate_rigid_bodies");
	REGISTER_SYSTEM(JointInformation, "joint_information");
	REGISTER_SYSTEM(ParticleSimulator, "particle_simulator");
	REGISTER_SYSTEM(GUI, "gui");
	REGISTER_SYSTEM(Audio, "audio");
	REGISTER_SYSTEM(Lights, "lights");

	REGISTER_SYSTEM(Shadows, "shadows");
	REGISTER_SYSTEM(CubemapRenderer, "cubemap_renderer");
	REGISTER_SYSTEM(RenderHeightmap, "render_heightmap");
	REGISTER_SYSTEM(Renderer, "renderer");
	REGISTER_SYSTEM(WireframeRenderer, "wireframe_renderer");
	REGISTER_SYSTEM(DebugRenderer, "debug_renderer");
	REGISTER_SYSTEM(CollisionPrimitiveRenderer, "collision_primitive_renderer");
	REGISTER_SYSTEM(ParticleRenderer, "particle_renderer");
	REGISTER_SYSTEM(GUIRenderer, "gui_renderer");
	REGISTER_SYSTEM(PostProcessing, "post_processing");
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