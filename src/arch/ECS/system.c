#include "ECS/system.h"

#include "ECS/component.h"

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
		// For each entity in the first component table
		for (ComponentDataTableIterator itr = cdtGetIterator(
				 *(ComponentDataTable **)hashMapGetKey(
					 scene->componentTypes,
					 (UUID*)system->componentTypes.front->data));
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
				UUID *componentID = LIST_ITERATOR_GET_ELEMENT(UUID, litr);
				ComponentDataTable **table = hashMapGetKey(
					scene->componentTypes,
					componentID);

				if (!table || !*table || !componentID)
				{
					continue;
				}

				// Check if the entity exists in the table
				uint32 *entityIndex = cdtIteratorGetData(itr);

				if (!entityIndex)
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
