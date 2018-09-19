ffi.cdef[[
typedef struct progress_bar_component_t
{
	uint64 value;
	uint64 maxValue;
	kmVec3 color;
	kmVec3 backgroundColor;
	bool reversed;
} ProgressBarComponent;
]]

local component = engine.components:register("progress_bar", "ProgressBarComponent")
