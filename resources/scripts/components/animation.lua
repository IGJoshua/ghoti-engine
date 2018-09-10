ffi.cdef[[
typedef struct animation_component_t
{
    UUID skeleton;
    char idleAnimation[64];
    real32 speed;
	bool backwards;
	real64 transitionDuration;
} AnimationComponent;
]]

local component = engine.components:register("animation", "AnimationComponent")

local C = engine.C

function component:swapSkeleton(skeleton, idleAnimation, removeSkeleton, scene, speed, backwards, transitionDuration)
  if removeSkeleton or false then
    C.sceneRemoveEntity(scene.ptr, self.skeleton)
  end

  self.skeleton = skeleton
  self.idleAnimation = idleAnimation
  self.speed = speed or 1.0
  self.backwards = backwards or false
  self.transitionDuration = transitionDuration or 0.0
end