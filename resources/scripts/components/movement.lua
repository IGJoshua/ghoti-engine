ffi.cdef[[
typedef struct movement_component_t
{
    float speed;
    float maxSpeed;
    float jumpHeight;
} MovementComponent;
]]

local component = engine.components:register("movement", "MovementComponent")
