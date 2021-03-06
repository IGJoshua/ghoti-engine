#include "ECS/system.h"
#include "ECS/component.h"

#include "core/log.h"

#include "data/list.h"
#include "data/hash_map.h"

#include <string.h>

inline
System createSystem(
	List components,
	InitSystem init,
	BeginSystem begin,
	RunSystem run,
	EndSystem end,
	ShutdownSystem shutdown)
{
	System sys;
	sys.componentTypes = components;
	sys.init = init;
	sys.begin = begin;
	sys.run = run;
	sys.end = end;
	sys.shutdown = shutdown;
	return sys;
}

void systemRun(
	Scene *scene,
	System *system,
	real64 dt)
{
	if (system->begin)
	{
		system->begin(scene, dt);
	}

	if (system->run)
	{
		UUID *componentID = (UUID*)system->componentTypes.front->data;
		ComponentDataTable **cdt = (ComponentDataTable **)hashMapGetData(
			scene->componentTypes,
			componentID);

		if (!cdt || !*cdt)
		{
			LOG("ERROR: Component limit for the %s component "
				"is missing from the scene\n",
				componentID->string);
			ASSERT(false);
		}

		// For each entity in the first component table
		for (ComponentDataTableIterator itr = cdtGetIterator(*cdt);
			 !cdtIteratorAtEnd(itr);
			 cdtMoveIterator(&itr))
		{
			// Check to see if it has the other component types
			bool entityValid = true;

			ListIterator litr = listGetIterator(&system->componentTypes);
			for (listMoveIterator(&litr);
				 !listIteratorAtEnd(litr);
				 listMoveIterator(&litr))
			{
				// Get the component to check
				componentID = LIST_ITERATOR_GET_ELEMENT(UUID, litr);
				ComponentDataTable **table = hashMapGetData(
					scene->componentTypes,
					componentID);

				if (!table || !*table)
				{
					LOG("ERROR: Component limit for the %s component "
						"is missing from the scene\n",
						componentID->string);
					ASSERT(false);
				}
				else if (!componentID)
				{
					continue;
				}

				// Check if the entity exists in the table
				void *entityData = cdtGet(*table, cdtIteratorGetUUID(itr));

				if (!entityData)
				{
					entityValid = false;
					break;
				}
			}

			// Call the function
			if (entityValid)
			{
				system->run(
					scene,
					cdtIteratorGetUUID(itr),
					dt);
			}
		}
	}

	if (system->end)
	{
		system->end(scene, dt);
	}
}

void freeSystem(System *system)
{
	listClear(&system->componentTypes);
	system->init = 0;
	system->begin = 0;
	system->run = 0;
	system->end = 0;
	system->shutdown = 0;
}
