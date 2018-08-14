io.write("Loading general physics!\n")

ffi.cdef[[
typedef void *dBodyID;
typedef void *dSpaceID;
typedef void *dGeomID;
typedef void *dJointID;

typedef enum joint_type_e
{
	JOINT_TYPE_BALL_SOCKET = 0,
	JOINT_TYPE_HINGE,
	JOINT_TYPE_SLIDER
} JointType;

typedef struct ray_collision_t
{
    Bool hasContact;
    real32 contact_pos[3];
	real32 surface_normal[3];
	real32 distance;
    real32 minDist;
    UUID contact_UUID;
} RayCollision;

RayCollision rayCast(
    Scene *scene,
    kmVec3 pos,
    kmVec3 dir,
    real32 minDist,
    real32 length);
]]

io.write("General physics loaded!\n")
