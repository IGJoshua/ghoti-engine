#include "defines.h"

#include "audio/audio.h"

#include "ECS/ecs_types.h"

#include "data/data_types.h"
#include "data/list.h"

extern Scene* listenerScene;
extern List activeScenes;

void setListenerScene(const char *name)
{
    for (ListIterator itr = listGetIterator(&activeScenes);
         !listIteratorAtEnd(itr);
         listMoveIterator(&itr))
    {
        Scene *scene = *LIST_ITERATOR_GET_ELEMENT(Scene*, itr);
        if (!strcmp(name, scene->name))
        {
            listenerScene = scene;
            return;
        }
    }
}
