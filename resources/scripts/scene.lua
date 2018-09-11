local Scene = {}
local C = engine.C

Scene.ptr = ffi.cast("Scene *", 0)

function Scene:new(pScene)
  local scene = {}
  setmetatable(scene, self)

  scene.ptr = ffi.cast("Scene *", pScene)
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

function Scene:addComponentToEntity(component, entity, componentData)
  local componentID = C.idFromName(component)
  C.sceneAddComponentToEntity(self.ptr, entity, componentID, componentData)
end

function Scene:removeComponentFromEntity(component, entity)
  local componentID = C.idFromName(component)
  C.sceneRemoveComponentFromEntity(self.ptr, entity, componentID)
end

function Scene:removeEntity(entity)
  C.sceneRemoveEntity(self.ptr, entity)
end

function Scene:getComponentIterator(component)
  local itrOut = ffi.new("ComponentDataTableIterator[1]")
  local itr = ffi.new(
	"ComponentDataTableIterator",
	C.cdtGetIterator(
	  ffi.cast(
		"ComponentDataTable**",
		C.hashMapGetData(
		  self.ptr.componentTypes,
		  C.idFromName(component)))[0]))
  local first = true
  return function ()
	if not first then
	  itrOut[0] = itr
	  C.cdtMoveIterator(itrOut)
	  itr = itrOut[0]
	else
	  first = false
	end
	if C.cdtIteratorAtEnd(itr) == 0 then
	  return engine.components[component]:new(
			   C.cdtIteratorGetData(itr)),
			 C.cdtIteratorGetUUID(itr)
	end
  end
end

return Scene
