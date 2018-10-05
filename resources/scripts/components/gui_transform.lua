ffi.cdef[[
typedef enum pivot_e
{
	PIVOT_TOP_LEFT,
	PIVOT_TOP,
	PIVOT_TOP_RIGHT,
	PIVOT_LEFT,
	PIVOT_CENTER,
	PIVOT_RIGHT,
	PIVOT_BOTTOM_LEFT,
	PIVOT_BOTTOM,
	PIVOT_BOTTOM_RIGHT
} Pivot;

typedef enum layout_mode_e
{
	LAYOUT_MODE_NDC,
	LAYOUT_MODE_SCREEN
} LayoutMode;

typedef struct gui_transform_component_t
{
	kmVec2 position;
	kmVec2 size;
	Pivot pivot;
	LayoutMode mode;
} GUITransformComponent;
]]

local component = engine.components:register("gui_transform", "GUITransformComponent")