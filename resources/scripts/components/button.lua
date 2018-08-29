ffi.cdef[[
typedef struct button_component_t
{
	char text[1024];
	bool pressedLastFrame;
	bool pressed;
} ButtonComponent;
]]

local component = engine.components:register("button", "ButtonComponent")