ffi.cdef[[
typedef struct panel_component_t
{
	bool enabled;
	kmVec4 color;
	char font[64];
	uint32 fontSize;
	UUID widgetList;
} PanelComponent;
]]

local component = engine.components:register("panel", "PanelComponent")