ffi.cdef[[
typedef enum anchor_e
{
	ANCHOR_TOP_LEFT,
	ANCHOR_TOP,
	ANCHOR_TOP_RIGHT,
	ANCHOR_LEFT,
	ANCHOR_CENTER,
	ANCHOR_RIGHT,
	ANCHOR_BOTTOM_LEFT,
	ANCHOR_BOTTOM,
	ANCHOR_BOTTOM_RIGHT
} Anchor;

typedef struct gui_transform_component_t
{
	kmVec2 position;
	kmVec2 size;
	Anchor anchor;
} GUITransformComponent;
]]

local component = engine.components:register("gui_transform", "GUITransformComponent")