ffi.cdef[[
typedef struct transform_component_t
{
  kmQuaternion rotation;
  kmVec3 position;
  kmVec3 scale;
} TransformComponent;
]]

io.write("Defined Transform component for FFI\n")

local component = engine.components:register("transform", "TransformComponent")

component.numEntries = 256

io.write("Registered Transform component\n")
