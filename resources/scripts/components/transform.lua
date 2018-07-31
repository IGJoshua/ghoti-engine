ffi.cdef[[
typedef struct transform_component_t
{
  kmVec3 position;
  kmQuaternion rotation;
  kmVec3 scale;
  UUID parent;
  UUID firstChild;
  UUID nextSibling;
  Bool dirty;
  kmVec3 globalPosition;
  kmQuaternion globalRotation;
  kmVec3 globalScale;
  kmVec3 lastGlobalPosition;
  kmQuaternion lastGlobalRotation;
  kmVec3 lastGlobalScale;
} TransformComponent;
]]

io.write("Defined Transform component for FFI\n")

local component = engine.components:register("transform", "TransformComponent")

local emptyID = engine.C.idFromName("")

function component:markDirty(scene, entity)
  self.dirty = true

  local body = scene:getComponent("rigid_body", entity)
  if body then
	body.dirty = true
  end

  if ffi.C.strcmp(self.firstChild.string, emptyID.string) ~= 0 then
    local child = scene:getComponent("transform", self.firstChild)
    child:markDirty(scene)
    while ffi.C.strcmp(child.nextSibling.string, emptyID.string) ~= 0 do
      child = scene:getComponent("transform", child.nextSibling);
      child:markDirty(scene);
    end
  end
end

function component:parent(scene, child, parent)
  local parentTransform = scene:getComponent("transform", parent)

  self.nextSibling = parentTransform.firstChild
  parentTransform.firstChild = child

  self.parent = parent

  self:markDirty(scene, child)
end

io.write("Registered Transform component\n")
