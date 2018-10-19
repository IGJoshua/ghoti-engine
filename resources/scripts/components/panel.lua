ffi.cdef[[
typedef struct panel_component_t
{
	bool enabled;
	kmVec4 color;
	UUID firstWidget;
	uint32 layer;
} PanelComponent;
]]

local component = engine.components:register("panel", "PanelComponent")