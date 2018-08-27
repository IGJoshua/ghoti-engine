ffi.cdef[[
typedef enum text_alignment_e
{
	TEXT_ALIGNMENT_TOP_LEFT,
	TEXT_ALIGNMENT_TOP,
	TEXT_ALIGNMENT_TOP_RIGHT,
	TEXT_ALIGNMENT_LEFT,
	TEXT_ALIGNMENT_CENTER,
	TEXT_ALIGNMENT_RIGHT,
	TEXT_ALIGNMENT_BOTTOM_LEFT,
	TEXT_ALIGNMENT_BOTTOM,
	TEXT_ALIGNMENT_BOTTOM_RIGHT,
	TEXT_ALIGNMENT_WRAP
} TextAlignment;

typedef struct text_component_t
{
	char text[4096];
	kmVec4 color;
	TextAlignment alignment;
} TextComponent;
]]

local component = engine.components:register("text", "TextComponent")