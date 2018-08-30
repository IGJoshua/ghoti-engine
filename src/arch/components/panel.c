#include "components/panel.h"

#include "ECS/scene.h"

void removePanelWidgets(Scene *scene, PanelComponent *panel)
{
	UUID entity = panel->firstWidget;

	do
	{
		WidgetComponent *widget = sceneGetComponentFromEntity(
			scene,
			entity,
			idFromName("widget"));

		if (widget)
		{
			UUID widgetID = entity;
			entity = widget->nextWidget;
			sceneRemoveEntity(scene, widgetID);
		}
		else
		{
			break;
		}
	} while (true);
}