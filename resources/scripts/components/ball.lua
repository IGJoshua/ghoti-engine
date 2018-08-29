ffi.cdef[[
typedef struct ball_component_t
{
    real32 delay;
    kmVec2 velocity;
    kmVec4 bounds;
} BallComponent;
]]

local component = engine.components:register("ball", "BallComponent")
