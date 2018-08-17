ffi.cdef[[
typedef struct slider_joint_component_t
{
	dJointID id;
	kmVec3 axis;
} SliderJointComponent;
]]

local component = engine.components:register("slider_joint", "SliderJointComponent")
