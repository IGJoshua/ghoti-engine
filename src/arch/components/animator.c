#include "components/animator.h"

#include "asset_management/asset_manager_types.h"
#include "asset_management/model.h"

void playAnimation(
	ModelComponent *modelComponent,
	AnimatorComponent *animator,
	const char *name,
	bool loop,
	real32 speed,
	bool backwards)
{
	if (!strcmp(animator->currentAnimation, name))
	{
		return;
	}

	if (!modelComponent)
	{
		stopAnimation(animator);
		return;
	}

	Model *model = getModel(modelComponent->name);

	if (!model)
	{
		stopAnimation(animator);
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
		stopAnimation(animator);
		return;
	}

	strcpy(animator->currentAnimation, name);
	animator->time = backwards ? animation->duration : 0.0;
	animator->duration = animation->duration;
	animator->loop = loop;
	animator->speed = speed;
	animator->backwards = backwards;
	animator->paused = false;
}

void stopAnimation(AnimatorComponent *animator)
{
	if (animator)
	{
		strcpy(animator->currentAnimation, "");
		animator->time = 0.0;
		animator->duration = 0.0;
		animator->paused = false;
	}
}