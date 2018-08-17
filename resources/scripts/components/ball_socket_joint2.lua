ffi.cdef[[
typedef struct ball_socket2_joint_component_t
{
	dJointID id;
	kmVec3 anchor1;
	kmVec3 anchor2;
	real32 distance;
} BallSocket2JointComponent;
]]

local component = engine.components:register("ball_socket2_joint", "BallSocket2JointComponent")
