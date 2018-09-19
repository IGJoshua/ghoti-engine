ffi.cdef[[
typedef struct text_field_component_t
{
	char text[4096];
} TextFieldComponent;
]]

local component = engine.components:register("text_field", "TextFieldComponent")
