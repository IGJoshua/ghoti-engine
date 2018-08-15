ffi.cdef[[
typedef enum render_mode_e
{
	RENDER_MODE_FILL,
	RENDER_MODE_FILL_AND_LINES,
	RENDER_MODE_LINES,
	RENDER_MODE_POINTS,
	RENDER_MODE_FILL_AND_POINTS,
	RENDER_MODE_INVISIBLE
} RenderMode;

typedef struct model_component_t
{
	char name[1024];
	RenderMode renderMode;
} ModelComponent;
]]

io.write("Defined Model component for FFI\n")

local component = engine.components:register("model", "ModelComponent")

io.write("Registered Model component\n")
