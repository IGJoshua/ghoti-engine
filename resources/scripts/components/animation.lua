ffi.cdef[[
typedef struct animation_component_t
{
    UUID skeleton;
    char idleAnimation[64];
    real32 speed;
	real64 transitionDuration;
} AnimationComponent;
]]

local component = engine.components:register("animation", "AnimationComponent")

local C = engine.C

function component:removeSkeleton(scene)
  C.removeSkeleton(scene.ptr, self.skeleton)
end