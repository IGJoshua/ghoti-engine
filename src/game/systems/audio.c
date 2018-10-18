#include "defines.h"

#include "core/log.h"

#include "asset_management/audio.h"

#include "audio/audio.h"

#include "data/data_types.h"
#include "data/hash_map.h"
#include "data/list.h"

#include "ECS/ecs_types.h"
#include "ECS/scene.h"
#include "ECS/component.h"

#include "components/component_types.h"
#include "components/audio_source.h"

#include <AL/al.h>

internal UUID transformComponentID = {};
internal UUID rigidBodyComponentID = {};
internal UUID audioManagerComponentID = {};
internal UUID audioSourceComponentID = {};

internal uint32 audioSystemRefCount = 0;

#define PLAY_AUDIO_BUCKET_COUNT 1031

extern HashMap playAudioQueue;
extern Scene *listenerScene;
extern ALuint *g_Buffers;
extern ALuint *g_Sources;

internal int32 ptrcmp(void *a, void *b)
{
	return *(uint64*)a != *(uint64*)b;
}

internal
void initAudioSystem(Scene *scene)
{
	if (audioSystemRefCount == 0)
	{
		playAudioQueue = createHashMap(
			sizeof(AudioSourceComponent*),
			sizeof(PlayAudioQueueData),
			PLAY_AUDIO_BUCKET_COUNT,
			(ComparisonOp)&ptrcmp);

		if (!listenerScene)
		{
			listenerScene = scene;
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
	}

	audioSystemRefCount++;
}

internal
void beginAudioSystem(Scene *scene, real64 dt)
{
	if (audioSystemRefCount == 0 || !listenerScene)
	{
		return;
	}

	for (HashMapIterator itr = hashMapGetIterator(playAudioQueue);
		 !hashMapIteratorAtEnd(itr);)
	{
		AudioSourceComponent *audioSource =
			*(AudioSourceComponent**)hashMapIteratorGetKey(itr);

		PlayAudioQueueData *audio = hashMapIteratorGetValue(itr);
		audio->timer -= dt;

		bool remove = false;

		AudioFile audioData = getAudio(audio->name.string);
		if (strlen(audioData.name.string) > 0)
		{
			strcpy(audioSource->currentAudio, audio->name.string);

			alSourceRewind(g_Sources[audioSource->id]);

			alSourcei(
				g_Sources[audioSource->id],
				AL_BUFFER,
				g_Buffers[audioData.id]);

			alSourcePlay(g_Sources[audioSource->id]);

			remove = true;
		}
		else if (audio->timer < 0.0)
		{
			remove = true;
		}

		if (remove)
		{
			hashMapMoveIterator(&itr);
			hashMapDelete(playAudioQueue, &audioSource);
		}
		else
		{
			hashMapMoveIterator(&itr);
		}
	}

	AudioSourceComponent * sourceComp;
	TransformComponent * transformComp;
	RigidBodyComponent * rigidBodyComp;

	uint32 sourceID = 0;
	for (ComponentDataTableIterator itr = cdtGetIterator(
			 *(ComponentDataTable **)hashMapGetData(
				 scene->componentTypes,
				 &audioSourceComponentID));
		 !cdtIteratorAtEnd(itr) && sourceID < NUM_AUDIO_SRC;
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
	if (audioSystemRefCount == 0 || !listenerScene)
	{
		return;
	}

	AudioSourceComponent * sourceComp;
	TransformComponent * transformComp;
	RigidBodyComponent * rigidBodyComp;

	sourceComp = sceneGetComponentFromEntity(
		scene,
		entityID,
		audioSourceComponentID);

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

	if (isSourceActive(sourceComp))
	{
		getAudio(sourceComp->currentAudio);
	}
	else
	{
		strcpy(sourceComp->currentAudio, "");
	}
}

internal
void shutdownAudioSystem(Scene *scene)
{
	if (--audioSystemRefCount == 0)
	{
		stopAllAudio();
		freeHashMap(&playAudioQueue);
	}
}

System createAudioSystem(void)
{
	transformComponentID = idFromName("transform");
	rigidBodyComponentID = idFromName("rigid_body");
	audioManagerComponentID = idFromName("audio_manager");
	audioSourceComponentID = idFromName("audio_source");

	System sys = {};

	sys.componentTypes = createList(sizeof(UUID));
	listPushFront(&sys.componentTypes, &audioSourceComponentID);

	sys.init = &initAudioSystem;
	sys.begin = &beginAudioSystem;
	sys.run = &runAudioSystem;
	sys.shutdown = &shutdownAudioSystem;

	return sys;
}
