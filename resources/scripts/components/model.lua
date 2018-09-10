ffi.cdef[[
typedef struct model_component_t
{
    char name[64];
    bool visible;
} ModelComponent;
]]

local component = engine.components:register("model", "ModelComponent")

local C = engine.C

function component:swap(name, removeAnimation, scene, uuid, skeleton, idleAnimation, removeSkeleton, speed, transitionDuration)
  C.freeModel(self.name)
  self.name = name
  C.loadModel(name)

  if removeAnimation or false then
    C.sceneRemoveComponentFromEntity(scene.ptr, uuid, C.idFromName("animation"))
  elseif scene then
    local animation = scene:getComponent("animation", uuid)
    if animation then
      animation:swapSkeleton(skeleton, idleAnimation, removeSkeleton, scene, speed, transitionDuration)
    end
  end
end