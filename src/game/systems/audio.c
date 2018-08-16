#include "defines.h"

#include "core/log.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/efx-creative.h>
#include <AL/efx.h>
#include <AL/efx-presets.h>

#define NUM_BUFF 4

internal UUID transformComponentID = {};
internal UUID rigidBodyComponentID = {};
internal UUID collisionComponentID = {};
internal UUID collisionTreeNodeComponentID = {};
internal UUID hitInformationComponentID = {};
internal UUID hitListComponentID = {};
internal UUID surfaceInformationComponentID = {};

internal ALCdevice * device = NULL;
internal ALCcontext * context = NULL;
internal ALCboolean g_bEAX = 0;
internal ALuint g_Buffers[NUM_BUFF] = {};
internal ALenum errorCode = 0;

internal
void initAudioSystem(Scene *scene)
{
    //Init
    device = alcOpenDevice(0);

    if(device)
    {
        context = alcCreateContext(device, 0);
    }

    //Check for EAX 2.0
    g_bEAX = alIsExtensionPresent("EAX2.0");

    //Creat Buffers
    alGetError();

    alGenBuffers(NUM_BUFF, g_Buffers);

    if((errorCode = alGetError()) != AL_NO_ERROR)
    {
        printf("alGenBuffers has errored: %i", errorCode);
        return;
    }




}

internal
void beginAudioSystem(Scene *scene, real64 dt)
{

}

internal
void runAudioSystem(Scene *scene, UUID entityID, real64 dt)
{

}

internal
void shutdownAudioSystem(Scene *scene)
{
    context=alcGetCurrentContext();
    device=alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

System createAudioSystem(void)
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
	//listPushFront(&sys.componentTypes, &transformComponentID);

	sys.init = &initAudioSystem;
	sys.begin = &beginAudioSystem;
	sys.run = &runAudioSystem;
	sys.shutdown = &shutdownAudioSystem;

	return sys;
}
