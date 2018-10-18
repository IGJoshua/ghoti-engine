#include "components/rigid_body.h"
#include "components/ray_casting.h"
#include "components/transform.h"

#include "core/log.h"

#include <ode/ode.h>

internal
void nearCallback(void *data, dGeomID o1, dGeomID o2)
{
    if (dGeomIsSpace(o1) || dGeomIsSpace(o2))
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

        if (!hasContact)
        {
            return;
        }

        RayCollision* ray = (RayCollision*)data;

        if (contact.depth < ray->minDist)
        {
            return;
        }

        ray->hasContact = true;

        if (contact.depth < ray->distance)
        {
            ray->contact_pos[0] = contact.pos[0];
            ray->contact_pos[1] = contact.pos[1];
            ray->contact_pos[2] = contact.pos[2];

            ray->surface_normal[0] = contact.normal[0];
            ray->surface_normal[1] = contact.normal[1];
            ray->surface_normal[2] = contact.normal[2];

            ray->distance = contact.depth;

            UUID * contactID = dGeomGetData(o2);

            memcpy(&ray->contact_UUID.string, &contactID->string, sizeof(UUID));
        }
    }
}

RayCollision rayCast(
    Scene *scene,
    kmVec3 pos,
    kmVec3 dir,
    real32 minDist,
    real32 length)
{
    RayCollision rayInfo = {};
    rayInfo.minDist = minDist;
	rayInfo.distance = length;

    dGeomID ray = dCreateRay(scene->physicsSpace, length);
    dGeomRaySet(ray, pos.x, pos.y, pos.z, dir.x, dir.y, dir.z);

    dGeomRaySetBackfaceCull(ray, 1);
    dGeomRaySetClosestHit(ray, 1);

    dSpaceCollide2(ray, (dGeomID)scene->physicsSpace, &rayInfo, &nearCallback);

    dGeomDestroy(ray);

    return rayInfo;
}
