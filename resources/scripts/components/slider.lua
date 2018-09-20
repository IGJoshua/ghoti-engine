ffi.cdef[[
typedef struct slider_component_t
{
	int32 value;
	int32 minValue;
	int32 maxValue;
	int32 stepSize;
	real32 height;
	real32 length;
	real32 buttonSize;
	kmVec4 fillColor;
	kmVec4 backgroundColor;
	kmVec4 buttonColor;
} SliderComponent;
]]

local component = engine.components:register("slider", "SliderComponent")