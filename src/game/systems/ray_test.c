#include "defines.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"

#include "components/component_types.h"
#include "components/rigid_body.h"
#include "components/transform.h"

#include <ode/ode.h>

#include <math.h>

internal UUID transformComponentID = {};
internal UUID rigidBodyComponentID = {};
internal UUID collisionComponentID = {};
internal UUID collisionTreeNodeComponentID = {};
internal UUID hitInformationComponentID = {};
internal UUID hitListComponentID = {};
internal UUID surfaceInformationComponentID = {};

internal
void runRayTestSystem(Scene *scene, UUID entityID, real64 dt)
{
	// update the rest of the rigidbody information

    if(!strcmp(entityID.string, "ray_caster"))
    {
        return;
    }
}

System createRayTestSystem(void)
{
	transformComponentID = idFromName("transform");
	rigidBodyComponentID = idFromName("rigid_body");
	collisionTreeNodeComponentID = idFromName("collision_tree_node");
	collisionComponentID = idFromName("collision");
	hitInformationComponentID = idFromName("hit_information");
	hitListComponentID = idFromName("hit_list");
	surfaceInformationComponentID = idFromName("surface_information");

	System sys = {};

	sys.componentTypes = createList(sizeof(UUID));
	listPushFront(&sys.componentTypes, &transformComponentID);
	listPushFront(&sys.componentTypes, &rigidBodyComponentID);
	listPushFront(&sys.componentTypes, &collisionComponentID);

	sys.init = NULL;
	sys.begin = NULL;
	sys.run = &runRayTestSystem;
	sys.shutdown = NULL;

	return sys;
}
