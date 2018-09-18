#pragma once
#include "defines.h"

struct GLFWwindow *initWindow(uint32 width, uint32 height, const char *title);

bool getVSYNCMode(void);
void switchVSYNCMode(void);
void setVSYNCMode(bool vsync);

bool getFullscreenMode(void);
void switchFullscreenMode(void);
void setFullscreenMode(bool fullscreen);

int32 closeWindow(void);
int32 freeWindow(struct GLFWwindow *window);