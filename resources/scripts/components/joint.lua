ffi.cdef[[
typedef struct joint_component_t
{
	char name[64];
} JointComponent;
]]

local component = engine.components:register("joint", "JointComponent")
