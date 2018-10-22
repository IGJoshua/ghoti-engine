ffi.cdef[[
typedef struct joint_information_component_t
{
	JointType type;
	UUID object1;
	UUID object2;
} JointInformationComponent;
]]

local component = engine.components:register("joint_information", "JointInformationComponent")
