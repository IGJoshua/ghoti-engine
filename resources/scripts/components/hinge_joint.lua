ffi.cdef[[

typedef struct hinge_joint_component_t
{
	dJointID id;
	kmVec3 anchor;
	kmVec3 axis;
} HingeJointComponent;
]]

local component = engine.components:register("hinge_joint", "HingeJointComponent")
