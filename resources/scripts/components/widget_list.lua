ffi.cdef[[
typedef struct widget_list_component_t
{
	UUID nextWidget;
} WidgetListComponent;
]]

local component = engine.components:register("widget_list", "WidgetListComponent")