ffi.cdef[[
typedef struct progress_bar_component_t
{
	real32 value;
	kmVec4 color;
	kmVec4 backgroundColor;
	bool reversed;
} ProgressBarComponent;
]]

local component = engine.components:register("progress_bar", "ProgressBarComponent")
