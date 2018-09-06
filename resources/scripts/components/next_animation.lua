ffi.cdef[[
typedef struct next_animation_component_t
{
	char name[64];
	bool loop;
	real32 speed;
	bool backwards;
} NextAnimationComponent;
]]

local component = engine.components:register("next_animation", "NextAnimationComponent")

function component:set(name, loop, speed, backwards)
  self.name = name
  self.loop = loop or false
  self.speed = speed or 1.0
  self.backwards = backwards or false
end