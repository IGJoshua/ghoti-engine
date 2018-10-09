ffi.cdef[[
typedef enum light_type_e
{
	LIGHT_TYPE_DIRECTIONAL,
	LIGHT_TYPE_POINT,
	LIGHT_TYPE_SPOT
} LightType;

typedef struct light_component_t
{
	bool enabled;
	LightType type;
	kmVec3 radiantFlux;
	real32 radius;
	kmVec2 size;
} LightComponent;
]]

local component = engine.components:register("light", "LightComponent")