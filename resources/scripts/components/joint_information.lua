ffi.cdef[[
typedef enum joint_type_e
{
	JOINT_TYPE_BALL_SOCKET = 0,
	JOINT_TYPE_HINGE,
	JOINT_TYPE_HINGE2,
	JOINT_TYPE_SLIDER,
	JOINT_TYPE_BALL_SOCKET2
} JointType;

typedef struct joint_information_component_t
{
	JointType type;
	UUID object1;
	UUID object2;
} JointInformationComponent;
]]

local component = engine.components:register("joint_information", "JointInformationComponent")
