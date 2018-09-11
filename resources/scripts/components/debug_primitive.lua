ffi.cdef[[
typedef enum debug_primitive_type_e
{
	DEBUG_PRIMITIVE_TYPE_POINT,
	DEBUG_PRIMITIVE_TYPE_LINE,
	DEBUG_PRIMITIVE_TYPE_TRANSFORM
} DebugPrimitiveType;

typedef struct debug_primitive_t
{
	bool visible;
	DebugPrimitiveType type;
	real32 size;
	kmVec3 color;
	UUID endpoint;
	kmVec3 endpointColor;
} DebugPrimitiveComponent;
]]

local component = engine.components:register("debug_primitive", "DebugPrimitiveComponent")