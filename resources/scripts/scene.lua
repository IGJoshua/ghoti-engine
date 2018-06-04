local Scene = {}
local C = engine.C

Scene.ptr = ffi.cast("Scene *", 0)

function Scene:new(pScene)
  local scene = {}
  setmetatable(scene, self)

  scene.ptr = ffi.cast("Scene *", pScene)
  scene.physicsSystems = {}
  scene.renderSystems = {}
  self.__index = self

  return scene
end

function Scene:getComponent(component, entity)
  if engine.components[component] then
	local componentID = C.idFromName(component)
	local componentData = C.sceneGetComponentFromEntity(self.ptr, entity, componentID)
	return engine.components[component]:new(componentData)
  else
	io.write(string.format("Attempting to get undefined component type %s\n", component))
	return nil
  end
end

return Scene
