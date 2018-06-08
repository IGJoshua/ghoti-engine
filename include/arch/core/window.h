#pragma once
#include "defines.h"

struct GLFWwindow *initWindow(uint32 width, uint32 height, const char *title);
int32 closeWindow();
int32 freeWindow(struct GLFWwindow *window);
