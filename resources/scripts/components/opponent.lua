ffi.cdef[[
typedef struct opponent_component_t
{
    real32 speed;
    UUID target;
} OpponentComponent;
]]

local component = engine.components:register("opponent", "OpponentComponent")
