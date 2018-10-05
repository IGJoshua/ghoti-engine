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

typedef enum position_layout_mode_e
{
	POSITION_LAYOUT_MODE_NDC,
	POSITION_LAYOUT_MODE_SCREEN
} PositionLayoutMode;

typedef enum size_layout_mode_e
{
	SIZE_LAYOUT_MODE_RATIO,
	SIZE_LAYOUT_MODE_PIXELS
} SizeLayoutMode;

typedef struct gui_transform_component_t
{
	kmVec2 position;
	PositionLayoutMode positionMode;
	kmVec2 size;
	SizeLayoutMode sizeMode;
	Pivot pivot;
} GUITransformComponent;
]]

local component = engine.components:register("gui_transform", "GUITransformComponent")