ffi.cdef[[
typedef struct next_animation_component_t
{
	char name[64];
	int32 loopCount;
	real32 speed;
	real64 transitionDuration;
} NextAnimationComponent;
]]

local component = engine.components:register("next_animation", "NextAnimationComponent")

function component:set(name, loopCount, speed, transitionDuration)
  self.name = name
  self.loopCount = loopCount or 0
  self.speed = speed or 1.0
  self.transitionDuration = transitionDuration or 0.0
end