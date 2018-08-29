#include "components/panel.h"

#include "ECS/scene.h"

void removePanelWidgets(Scene *scene, PanelComponent *panel)
{
	UUID entity = panel->widgetList;

	do
	{
		WidgetListComponent *widgetList = sceneGetComponentFromEntity(
			scene,
			entity,
			idFromName("widget_list"));

		if (widgetList)
		{
			UUID widget = entity;
			entity = widgetList->nextWidget;
			sceneRemoveEntity(scene, widget);
		}
		else
		{
			break;
		}
	} while (true);
}