#pragma once
#include "defines.h"

struct GLFWwindow *initWindow(uint32 width, uint32 height, const char *title);
int32 closeWindow(void);
int32 freeWindow(struct GLFWwindow *window);
