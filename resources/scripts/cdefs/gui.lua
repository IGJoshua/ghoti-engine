ffi.cdef[[

typedef enum gui_button_e
{
	GUI_BUTTON_LEFT,
	GUI_BUTTON_MIDDLE,
	GUI_BUTTON_RIGHT
} GUIButton;

void guiSetMousePosition(int32 x, int32 y);
void guiClickMouse(GUIButton button, bool pressed);

]]