#include "components/animation.h"

TransformComponent* getJointTransform(
	Scene *scene,
	UUID joint,
	const char *name)
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
				name);

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