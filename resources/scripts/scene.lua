local Scene = {}

local mt = {}

function Scene.new(pScene)
  local scene = {}
  setmetatable(scene, mt)

  scene.ptr = ffi.cast("Scene *", pScene)
  scene.physicsSystems = {}
  scene.renderSystems = {}

  return scene
end

function Scene:getComponent(component, entity)
end

return Scene
