ffi.cdef[[
typedef enum camera_projection_type_e
{
	CAMERA_PROJECTION_TYPE_PERSPECTIVE = 0,
	CAMERA_PROJECTION_TYPE_ORTHOGRAPHIC,
	CAMERA_PROJECTION_TYPE_COUNT
} CameraProjectionType;

typedef struct camera_component_t
{
	real32 nearPlane;
	real32 farPlane;
	real32 aspectRatio;
	real32 fov;
	real32 bounds[4];
	CameraProjectionType projectionType;
} CameraComponent;
]]

local component = engine.components:register("camera", "CameraComponent")