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

io.write("Defined Oscillator component for FFI\n")

local component = engine.components:register("oscillator", "OscillatorComponent")

io.write("Registered Oscillator component\n")
