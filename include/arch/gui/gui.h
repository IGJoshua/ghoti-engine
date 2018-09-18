#pragma once
#include "defines.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING

#include <nuklear/nuklear.h>

typedef enum gui_button_e
{
	GUI_BUTTON_LEFT,
	GUI_BUTTON_MIDDLE,
	GUI_BUTTON_RIGHT
} GUIButton;

void guiSetMousePosition(int32 x, int32 y);
void guiClickMouse(GUIButton button, bool pressed);