ffi.cdef[[
typedef struct movement_component_t
{
	real32 speed;
	real32 maxSpeed;
	real32 jumpHeight;
} MovementComponent;
]]

local component = engine.components:register("movement", "MovementComponent")
