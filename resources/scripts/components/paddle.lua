ffi.cdef[[
typedef struct paddle_component_t
{
    kmVec3 bounds;
} PaddleComponent;
]]

local component = engine.components:register("paddle", "PaddleComponent")
