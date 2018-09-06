#include "components/animation.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/model.h"

TransformComponent* getJointTransform(
	Scene *scene,
	UUID joint,
	const char *name,
	UUID *uuid)
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
		return NULL;
	}

	if (jointComponent && !strcmp(name, jointComponent->name))
	{
		if (uuid)
		{
			*uuid = joint;
		}

		return transform;
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
			TransformComponent *jointTransform = getJointTransform(
				scene,
				child,
				name,
				uuid);

			if (jointTransform)
			{
				return jointTransform;
			}
		}
		else
		{
			break;
		}

		child = transform->nextSibling;
	} while (true);

	return NULL;
}

void playAnimation(
	ModelComponent *modelComponent,
	AnimationComponent *animationComponent,
	const char *name,
	bool loop,
	real32 speed,
	bool backwards)
{
	if (!modelComponent)
	{
		stopAnimation(animationComponent);
		return;
	}

	Model *model = getModel(modelComponent->name);

	if (!model)
	{
		stopAnimation(animationComponent);
		return;
	}

	Animation *animation = NULL;
	for (uint32 i = 0; i < model->numAnimations; i++)
	{
		if (!strcmp(model->animations[i].name.string, name))
		{
			animation = &model->animations[i];
			break;
		}
	}

	if (!animation)
	{
		stopAnimation(animationComponent);
		return;
	}

	strcpy(animationComponent->name, name);
	animationComponent->time = backwards ? animation->duration : 0.0;
	animationComponent->duration = animation->duration;
	animationComponent->loop = loop;
	animationComponent->speed = speed;
	animationComponent->backwards = backwards;
	animationComponent->paused = false;
}

void stopAnimation(AnimationComponent *animationComponent)
{
	if (animationComponent)
	{
		strcpy(animationComponent->name, "");
		animationComponent->time = 0.0;
		animationComponent->duration = 0.0;
		animationComponent->paused = false;
	}
}