ffi.cdef[[
typedef struct model_component_t
{
	char name[64];
	bool visible;
} ModelComponent;
]]

local component = engine.components:register("model", "ModelComponent")

local C = engine.C

function component:changeModel(name)
  C.freeModel(self.name)
  self.name = name
  C.loadModel(name)
end