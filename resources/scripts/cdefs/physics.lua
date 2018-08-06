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
]]
