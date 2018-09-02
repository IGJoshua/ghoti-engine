ffi.cdef[[
typedef struct animation_component_t
{
	char name[64];
	UUID skeleton;
} AnimationComponent;
]]

local component = engine.components:register("animation", "AnimationComponent")