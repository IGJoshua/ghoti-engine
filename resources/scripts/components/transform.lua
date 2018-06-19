ffi.cdef[[
typedef struct transform_component_t
{
  kmVec3 position;
  kmQuaternion rotation;
  kmVec3 scale;
  UUID parent;
  bool dirty;
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

io.write("Registered Transform component\n")
