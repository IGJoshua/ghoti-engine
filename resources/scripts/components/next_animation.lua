ffi.cdef[[
typedef struct next_animation_component_t
{
	char name[64];
	int32 loopCount;
	real32 speed;
	bool backwards;
} NextAnimationComponent;
]]

local component = engine.components:register("next_animation", "NextAnimationComponent")

function component:set(name, loopCount, speed, backwards)
  self.name = name
  self.loopCount = loopCount or 0
  self.speed = speed or 1.0
  self.backwards = backwards or false
end