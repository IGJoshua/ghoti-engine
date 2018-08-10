#include "components/rigid_body.h"
#include "components/ray_casting.h"
#include "components/transform.h"

#include "core/log.h"

#include <ode/ode.h>

internal
void nearCallback(void *data, dGeomID o1, dGeomID o2)
{
    if(dGeomIsSpace(o1) || dGeomIsSpace(o2))
    {
        dSpaceCollide2(o1, o2, data, &nearCallback);
    }
    else
    {
        dContactGeom contact = {};

        int32 hasContact = dCollide(
			o1,
			o2,
			1,
			&contact,
			sizeof(dContactGeom));

        //dGeomGetData(dGeomID geom)
    }
}

void rayCast(Scene *scene, kmVec3 pos, kmVec3 dir, real32 length)
{
    //TODO: Make an array(like 30 items) of collision data (new structure)

    dGeomID ray = dCreateRay(scene->physicsSpace, length);
    dGeomRaySet(ray, pos.x, pos.y, pos.z, dir.x, dir.y, dir.z);

    dSpaceCollide2(ray, (dGeomID)scene->physicsSpace, NULL, &nearCallback);
}
