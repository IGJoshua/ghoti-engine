ffi.cdef[[
typedef struct animation_component_t
{
	char name[64];
	UUID skeleton;
	real64 time;
	real64 duration;
} AnimationComponent;
]]

local component = engine.components:register("animation", "AnimationComponent")