ffi.cdef[[
typedef struct model_component_t
{
  char name[1024];
} ModelComponent;
]]

io.write("Defined Model component for FFI\n")

local component = engine.components:register("model", "ModelComponent")

component.numEntries = 256

io.write("Registered Model component\n")
