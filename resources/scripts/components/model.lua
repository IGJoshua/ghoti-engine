ffi.cdef[[
typedef struct model_component_t
{
  char name[1024];
  bool visible;
} ModelComponent;
]]

io.write("Defined Model component for FFI\n")

local component = engine.components:register("model", "ModelComponent")

io.write("Registered Model component\n")
