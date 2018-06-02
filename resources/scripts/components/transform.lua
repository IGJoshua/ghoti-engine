ffi.cdef[[
typedef struct transform_component_t
{
  kmQuaternion rotation;
  kmVec3 position;
  kmVec3 scale;
} TransformComponent;
]]

io.write("Defined Transform component for FFI\n")

io.write(string.format("Size of transform component: %d\n", ffi.sizeof("TransformComponent")))

local component = engine.components:register("transform", "TransformComponent")

io.write("Registered Transform component\n")
