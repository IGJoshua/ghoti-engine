ffi.cdef[[
typedef struct hinge2_joint_component_t
{
	dJointID id;
	kmVec3 anchor;
	kmVec3 axis1;
	kmVec3 axis2;
} Hinge2JointComponent;
]]

local component = engine.components:register("hinge2_joint", "Hinge2JointComponent")