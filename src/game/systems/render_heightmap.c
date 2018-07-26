#include "defines.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"

#include "asset_management/asset_manager_types.h"

#include "renderer/renderer_types.h"

#include "components/component_types.h"
#include "components/rigid_body.h"
#include "components/transform.h"

#include <ode/ode.h>

#define SCENE_BUCKET_COUNT 7

internal UUID heightmapComponentID = {};

internal bool rendererActive = false;

internal HashMap heightmapModels = 0;

void initRenderHeightmapSystem(Scene *scene)
{
	if (!rendererActive)
	{
		// TODO: create and compile the shader pipeline
	}

	// TODO: iterate over all the heightmaps and load the images and create geometry for it
}

void shutdownRenderHeightmapSystem(Scene *scene)
{
	// TODO: free any information in the hash map with the scene, and delete the hash map entry
}

internal
int32 ptrEq(void *thing1, void *thing2)
{
	return *(uint64*)thing1 != *(uint64*)thing2;
}

System createRenderHeightmapSystem(void)
{
	heightmapComponentID = idFromName("heightmap");

	heightmapModels = createHashMap(sizeof(Scene *), sizeof(Mesh), SCENE_BUCKET_COUNT, &ptrEq);

	System ret = {};

	ret.componentTypes = createList(sizeof(UUID));
	listPushFront(&ret.componentTypes, &heightmapComponentID);

	return ret;
}
