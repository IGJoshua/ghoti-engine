ffi.cdef[[
typedef struct widget_component_t
{
	bool enabled;
	UUID nextWidget;
} WidgetComponent;
]]

local component = engine.components:register("widget", "WidgetComponent")