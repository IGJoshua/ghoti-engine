#include "components/widget.h"
#include "components/component_types.h"

#include "data/hash_map.h"

#include "ECS/scene.h"
#include "ECS/component.h"

void removeWidget(Scene *scene, UUID entity)
{
	UUID panelComponentID = idFromName("panel");
	UUID widgetComponentID = idFromName("widget");

	ComponentDataTable *panelComponents =
		*(ComponentDataTable**)hashMapGetData(
				scene->componentTypes,
				&panelComponentID);

	bool removed = false;

	for (ComponentDataTableIterator itr = cdtGetIterator(panelComponents);
			!cdtIteratorAtEnd(itr);
			cdtMoveIterator(&itr))
	{
		PanelComponent *panelComponent = cdtIteratorGetData(itr);
		UUID widgetID = panelComponent->firstWidget;

		WidgetComponent *widgetComponent = sceneGetComponentFromEntity(
			scene,
			widgetID,
			widgetComponentID);

		if (!strcmp(entity.string, widgetID.string))
		{
			panelComponent->firstWidget = idFromName("");
			if (widgetComponent)
			{
				panelComponent->firstWidget = widgetComponent->nextWidget;
			}

			removed = true;
		}
		else
		{
			while (widgetComponent)
			{
				WidgetComponent *previousWidgetComponent = widgetComponent;

				widgetID = widgetComponent->nextWidget;
				widgetComponent = sceneGetComponentFromEntity(
					scene,
					widgetID,
					widgetComponentID);

				if (!strcmp(
					entity.string,
					previousWidgetComponent->nextWidget.string))
				{
					previousWidgetComponent->nextWidget = idFromName("");
					if (widgetComponent)
					{
						previousWidgetComponent->nextWidget =
							widgetComponent->nextWidget;
					}

					removed = true;
					break;
				}
			}
		}

		if (removed)
		{
			break;
		}
	}
}