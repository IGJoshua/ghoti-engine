ffi.cdef[[
typedef struct player_component_t
{
    real32 speed;
} PlayerComponent;
]]

local component = engine.components:register("player", "PlayerComponent")
