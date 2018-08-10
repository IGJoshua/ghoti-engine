ffi.cdef[[
typedef struct ball_socket_joint_component_t
{
	dJointID id;
	kmVec3 anchor;
} BallSocketJointComponent;
]]

local component = engine.components:register("ball_socket_joint", "BallSocketJointComponent")
