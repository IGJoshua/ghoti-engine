#include "defines.h"

#include "core/log.h"

#include "asset_management/audio.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"

#include "components/component_types.h"
#include "components/audio.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include <AL/alext.h>
#include <AL/efx-creative.h>
#include <AL/efx.h>
#include <AL/efx-presets.h>

internal UUID transformComponentID = {};
internal UUID rigidBodyComponentID = {};
internal UUID audioManagerComonentID = {};
internal UUID audioSourceComonentID = {};

ALCdevice *device = NULL;
ALCcontext *context = NULL;
ALCboolean g_bEAX = 0;
ALuint *g_Buffers = 0;
ALuint *g_Sources = 0;
ALenum errorCode = 0;
uint64 freeBuffer = 0;
uint64 NUM_BUFF = 32;
uint64 NUM_SRC = 32;
bool system_setup = false;

extern Scene *listenerScene;

internal
void initAudioSystem(Scene *scene)
{
    if (system_setup)
    {
        return;
    }

    //Init
    device = alcOpenDevice(NULL);

    if (device)
    {
        context = alcCreateContext(device, NULL);

        if (context)
        {
            alcMakeContextCurrent(context);
        }
        else
        {
            LOG("Could not create a context\n");
            return;
        }
    }
    else
    {
        LOG("Could not create device\n");
        return;
    }

    //Check for EAX 2.0
    g_bEAX = alIsExtensionPresent("EAX2.0");

    //Creat Buffers
    alGetError();

    g_Buffers = malloc(sizeof(ALuint)*NUM_BUFF);
    alGenBuffers(NUM_BUFF, g_Buffers);

    errorCode = alGetError();
    if (errorCode != AL_NO_ERROR)
    {
        LOG("alGenBuffers has errored: %s\n", alGetString(errorCode));
        return;
    }

    // Generate Sources
    g_Sources = malloc(sizeof(ALuint)*NUM_SRC);
    alGenSources(NUM_SRC, g_Sources);

    errorCode = alGetError();
    if (errorCode != AL_NO_ERROR)
    {
        LOG("alGenSources has errored : %s\n", alGetString(errorCode));
        return;
    }

    system_setup = true;

    if (!listenerScene)
    {
        return;
    }

    TransformComponent * camTrans = sceneGetComponentFromEntity(
        scene,
        listenerScene->mainCamera,
        transformComponentID);

    ALfloat listenerPos[3]={};
    ALfloat	listenerOri[6]={0,0,0, 0,1,0};

    if (camTrans)
    {
        listenerPos[0] = camTrans->globalPosition.x;
        listenerPos[1] = camTrans->globalPosition.y;
        listenerPos[2] = camTrans->globalPosition.z;

        kmVec3 atVec, upVec;

        kmQuaternionGetForwardVec3RH(&atVec, &camTrans->globalRotation);

        kmQuaternionGetUpVec3(&upVec, &camTrans->globalRotation);

        listenerOri[0] = atVec.x;
        listenerOri[1] = atVec.y;
        listenerOri[2] = atVec.z;

        listenerOri[3] = upVec.x;
        listenerOri[4] = upVec.y;
        listenerOri[5] = upVec.z;

    }
    alListenerfv(AL_POSITION,listenerPos);
    alListenerfv(AL_ORIENTATION,listenerOri);

    RigidBodyComponent * camRigid = sceneGetComponentFromEntity(
        scene,
        listenerScene->mainCamera,
        rigidBodyComponentID);

    ALfloat listenerVel[3]={};
    if (camRigid)
    {
        listenerVel[0] = camRigid->velocity.x;
        listenerVel[1] = camRigid->velocity.y;
        listenerVel[2] = camRigid->velocity.z;
    }
    alListenerfv(AL_VELOCITY,listenerVel);

/*
    AudioFileComponent *audioEntity;

    for (ComponentDataTableIterator itr = cdtGetIterator(
			 *(ComponentDataTable **)hashMapGetData(
				 scene->componentTypes,
				 &audioFileComonentID));
		 !cdtIteratorAtEnd(itr);
		 cdtMoveIterator(&itr))
    {
        audioEntity = (AudioFileComponent *)cdtIteratorGetData(itr);
        loadAudio(audioEntity->name, audioEntity->looping);
    }
*/
    /*

    int32 channels;
	int32 sample_rate;
	int16 *output;
	int32 audio_size = stb_vorbis_decode_filename("resources/audio/BattlePrep.ogg", &channels, &sample_rate, &output);

    if (!output)
    {
        LOG("Audio Output is NULL");
        return;
    }

    ALenum format;

    if (channels == 1)
    {
        format = AL_FORMAT_MONO16;
    }
    else
    {
        format = AL_FORMAT_STEREO16;
    }

    //printf("channels: %d", channels);

    alBufferData(g_Buffers[0], format, output, audio_size * sizeof(int16) * channels, sample_rate);
    errorCode = alGetError();
    if (errorCode != AL_NO_ERROR)
    {
        LOG("alBufferData buffer 0 : %s\n", alGetString(errorCode));
        alDeleteBuffers(NUM_BUFF, g_Buffers);
        return;
    }


    // Attach buffer 0 to source
    alSourcei(g_Sources[0], AL_BUFFER, g_Buffers[0]);

    errorCode = alGetError();
    if (errorCode != AL_NO_ERROR)
    {
        LOG("alSourcei AL_BUFFER 0 : %s\n", alGetString(errorCode));
    }

    ALfloat source0Pos[]={ 0.0, 0.0, -2.0};	// Front and right of the listener
	ALfloat source0Vel[]={ 0.0, 0.0, 0.0};

    alSourcef(g_Sources[0],AL_PITCH,1.0f);
	alSourcef(g_Sources[0],AL_GAIN,1.0f);
	alSourcefv(g_Sources[0],AL_POSITION,source0Pos);
	alSourcefv(g_Sources[0],AL_VELOCITY,source0Vel);
    alSourcei(g_Sources[0],AL_LOOPING,AL_TRUE);

    alSourcePlay(g_Sources[0]);
    */
}

internal
void beginAudioSystem(Scene *scene, real64 dt)
{
    if (!system_setup || !listenerScene)
    {
        return;
    }

    AudioSourceComponent * sourceComp;
    TransformComponent * transformComp;
    RigidBodyComponent * rigidBodyComp;

    uint32 sourceID = 0;
    for (ComponentDataTableIterator itr = cdtGetIterator(
			 *(ComponentDataTable **)hashMapGetData(
				 scene->componentTypes,
				 &audioSourceComonentID));
		 !cdtIteratorAtEnd(itr) && sourceID < NUM_SRC;
		 cdtMoveIterator(&itr), sourceID++)
    {
        UUID entityID = cdtIteratorGetUUID(itr);

        sourceComp = (AudioSourceComponent *)cdtIteratorGetData(itr);

        sourceComp->id = sourceID;

        alSourcef(g_Sources[sourceID], AL_PITCH, sourceComp->pitch);
    	alSourcef(g_Sources[sourceID], AL_GAIN, sourceComp->gain);

        transformComp = sceneGetComponentFromEntity(
            scene,
            entityID,
            transformComponentID);

        ALfloat sourcePos[3] = {};

        if (transformComp)
        {
            sourcePos[0] = transformComp->globalPosition.x;
            sourcePos[1] = transformComp->globalPosition.y;
            sourcePos[2] = transformComp->globalPosition.z;
        }

        rigidBodyComp = sceneGetComponentFromEntity(
            scene,
            entityID,
            rigidBodyComponentID);

        ALfloat sourceVel[3]={};

        if (rigidBodyComp)
        {
            sourceVel[0] = rigidBodyComp->velocity.x;
            sourceVel[1] = rigidBodyComp->velocity.y;
            sourceVel[2] = rigidBodyComp->velocity.z;
        }

        alSourcefv(g_Sources[sourceID], AL_POSITION, sourcePos);
    	alSourcefv(g_Sources[sourceID], AL_VELOCITY, sourceVel);

        alSourcei(g_Sources[sourceID], AL_LOOPING, AL_FALSE);

        if (!strcmp(entityID.string, listenerScene->mainCamera.string))
        {
            ALfloat	listenerOri[6]={0,0,0, 0,1,0};

            if (transformComp)
            {
                kmVec3 atVec, upVec;

                kmQuaternionGetForwardVec3RH(
                    &atVec,
                    &transformComp->globalRotation);

                kmQuaternionGetUpVec3(&upVec, &transformComp->globalRotation);

                listenerOri[0] = atVec.x;
                listenerOri[1] = atVec.y;
                listenerOri[2] = atVec.z;

                listenerOri[3] = upVec.x;
                listenerOri[4] = upVec.y;
                listenerOri[5] = upVec.z;
            }

            alListenerfv(AL_POSITION,sourcePos);
            alListenerfv(AL_ORIENTATION,listenerOri);
            alListenerfv(AL_VELOCITY,sourceVel);
        }

    }
}

internal
void runAudioSystem(Scene *scene, UUID entityID, real64 dt)
{
    if (!system_setup || !listenerScene)
    {
        return;
    }

    AudioSourceComponent * sourceComp;
    TransformComponent * transformComp;
    RigidBodyComponent * rigidBodyComp;

    sourceComp = sceneGetComponentFromEntity(
        scene,
        entityID,
        audioSourceComonentID);

    uint32 sourceID = sourceComp->id;

    alSourcef(g_Sources[sourceID], AL_PITCH, sourceComp->pitch);
    alSourcef(g_Sources[sourceID], AL_GAIN, sourceComp->gain);
    alSourcef(g_Sources[sourceID], AL_LOOPING, sourceComp->looping);

    transformComp = sceneGetComponentFromEntity(
        scene,
        entityID,
        transformComponentID);

    ALfloat sourcePos[3] = {};

    if (transformComp)
    {
        sourcePos[0] = transformComp->globalPosition.x;
        sourcePos[1] = transformComp->globalPosition.y;
        sourcePos[2] = transformComp->globalPosition.z;
    }

    rigidBodyComp = sceneGetComponentFromEntity(
        scene,
        entityID,
        rigidBodyComponentID);

    ALfloat sourceVel[3]={};

    if (rigidBodyComp)
    {
        sourceVel[0] = rigidBodyComp->velocity.x;
        sourceVel[1] = rigidBodyComp->velocity.y;
        sourceVel[2] = rigidBodyComp->velocity.z;
    }

    alSourcefv(g_Sources[sourceID], AL_POSITION, sourcePos);
    alSourcefv(g_Sources[sourceID], AL_VELOCITY, sourceVel);

    if (!strcmp(entityID.string, listenerScene->mainCamera.string))
    {
        ALfloat	listenerOri[6]={0,0,0, 0,1,0};

        kmVec3 atVec, upVec;

        kmQuaternionGetForwardVec3RH(&atVec, &transformComp->globalRotation);

        kmQuaternionGetUpVec3(&upVec, &transformComp->globalRotation);

        listenerOri[0] = atVec.x;
        listenerOri[1] = atVec.y;
        listenerOri[2] = atVec.z;

        listenerOri[3] = upVec.x;
        listenerOri[4] = upVec.y;
        listenerOri[5] = upVec.z;

        alListenerfv(AL_POSITION,sourcePos);
        alListenerfv(AL_ORIENTATION,listenerOri);
        alListenerfv(AL_VELOCITY,sourceVel);
    }

}

internal
void shutdownAudioSystem(Scene *scene)
{
    if (!system_setup)
    {
        return;
    }

    system_setup = false;

    alSourceStopv(NUM_SRC, g_Sources);
    alDeleteBuffers(NUM_BUFF, g_Buffers);
    alDeleteSources(NUM_SRC, g_Sources);
    context = alcGetCurrentContext();
    device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);

/*
    AudioFileComponent *audioEntity;

    for (ComponentDataTableIterator itr = cdtGetIterator(
			 *(ComponentDataTable **)hashMapGetData(
				 scene->componentTypes,
				 &audioFileComonentID));
		 !cdtIteratorAtEnd(itr);
		 cdtMoveIterator(&itr))
    {
        audioEntity = (AudioFileComponent *)cdtIteratorGetData(itr);
        freeAudio(audioEntity->name);
    }
    */
}

System createAudioSystem(void)
{
	transformComponentID = idFromName("transform");
	rigidBodyComponentID = idFromName("rigid_body");
    audioManagerComonentID = idFromName("audio_manager");
    audioSourceComonentID = idFromName("audio_source");

	System sys = {};

	sys.componentTypes = createList(sizeof(UUID));
	listPushFront(&sys.componentTypes, &audioSourceComonentID);

	sys.init = &initAudioSystem;
	sys.begin = &beginAudioSystem;
	sys.run = &runAudioSystem;
	sys.shutdown = &shutdownAudioSystem;

	return sys;
}
