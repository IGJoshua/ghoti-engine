ffi.cdef[[
typedef struct joint_list_component_t
{
	UUID jointInfo;
	UUID next;
} JointListComponent;
]]

local component = engine.components:register("joint_list", "JointListComponent")
