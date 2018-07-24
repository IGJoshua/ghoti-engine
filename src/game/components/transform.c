#include "components/transform.h"

void tMarkDirty(Scene *scene, UUID entityID)
{
	UUID transformComponentID = idFromName("transform");

	TransformComponent *trans =
		sceneGetComponentFromEntity(scene, entityID, transformComponentID);

	if (trans)
	{
		trans->dirty = true;

		TransformComponent *child = 0;

		for (UUID currentChild = trans->firstChild;
			 strcmp(currentChild.string, "");
			 currentChild = child->nextSibling)
		{
			child = sceneGetComponentFromEntity(
				scene,
				currentChild,
				transformComponentID);

			tMarkDirty(scene, currentChild);
		}
	}
}
