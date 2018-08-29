ffi.cdef[[
typedef struct player_component_t
{
    real32 speed;
    real32 worldHeight;
} PlayerComponent;
]]

local component = engine.components:register("player", "PlayerComponent")
