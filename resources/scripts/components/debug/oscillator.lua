ffi.cdef[[
typedef struct oscillator_component_t
{
	kmVec3 position;
	kmVec3 direction;
	real32 time;
	real32 speed;
	real32 distance;
} OscillatorComponent;
]]

local component = engine.components:register("oscillator", "OscillatorComponent")