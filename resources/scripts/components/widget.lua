ffi.cdef[[
typedef struct widget_component_t
{
	bool enabled;
	kmVec4 backgroundColor;
	UUID nextWidget;
} WidgetComponent;
]]

local component = engine.components:register("widget", "WidgetComponent")