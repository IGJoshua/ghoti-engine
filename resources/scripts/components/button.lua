ffi.cdef[[
typedef struct button_component_t
{
	char text[1024];
	bool pressed;
	bool held;
	bool released;
	bool hovered;
} ButtonComponent;
]]

local component = engine.components:register("button", "ButtonComponent")