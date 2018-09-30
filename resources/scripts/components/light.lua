ffi.cdef[[
typedef enum light_type_e
{
	LIGHT_TYPE_DIRECTIONAL,
	LIGHT_TYPE_POINT,
	LIGHT_TYPE_SPOT
} LightType;

typedef struct light_component_t
{
	LightType type;
	kmVec3 color;
	kmVec3 ambient;
	real32 constantAttenuation;
	real32 linearAttenuation;
	real32 quadraticAttenuation;
	kmVec2 size;
} LightComponent;
]]

local component = engine.components:register("light", "LightComponent")