ffi.cdef[[
typedef enum camera_projection_type_e
{
  CAMERA_PROJECTION_TYPE_PERSPECTIVE = 0,
  CAMERA_PROJECTION_TYPE_ORTHOGRAPHIC,
  CAMERA_PROJECTION_TYPE_COUNT
} CameraProjectionType;

typedef struct camera_component_t
{
  float nearPlane;
  float farPlane;
  float aspectRatio;
  float fov;
  CameraProjectionType projectionType;
} CameraComponent;
]]

local component = engine.components:register("camera", "CameraComponent")

component.numEntries = 4
