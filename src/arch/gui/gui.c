#include "gui/gui.h"

#include "core/window.h"

#include <GLFW/glfw3.h>

uint32 guiRefCount = 0;
struct nk_context ctx;

void guiSetMousePosition(int32 x, int32 y)
{
	if (guiRefCount > 0)
	{
		nk_input_motion(&ctx, x, y);
	}
}

void guiClickMouse(GUIButton button, bool pressed)
{
	if (guiRefCount > 0)
	{
		nk_input_button(
			&ctx,
			(enum nk_buttons)button,
			ctx.input.mouse.pos.x,
			ctx.input.mouse.pos.y,
			pressed);
	}
}