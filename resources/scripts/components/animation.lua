ffi.cdef[[
typedef struct animation_component_t
{
    UUID skeleton;
    char idleAnimation[64];
    real32 speed;
    bool backwards;
} AnimationComponent;
]]

local component = engine.components:register("animation", "AnimationComponent")

local C = engine.C

function component:swapSkeleton(removeSkeleton, scene, skeleton, idleAnimation, speed, backwards)
  if removeSkeleton then
    C.sceneRemoveEntity(scene.ptr, self.skeleton)
  end

  self.skeleton = skeleton
  self.idleAnimation = idleAnimation
  self.speed = speed or 1.0
  self.backwards = backwards or false
end