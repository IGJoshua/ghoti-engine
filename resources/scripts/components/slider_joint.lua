ffi.cdef[[
typedef struct slider_joint_component_t
{
	dJointID slider;
	kmVec3 axis;
} SliderJointComponent;
]]

local component = engine.components:register("slider_joint", "SliderJointComponent")
