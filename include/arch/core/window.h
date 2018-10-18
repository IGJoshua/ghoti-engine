#pragma once
#include "defines.h"

struct GLFWwindow *initWindow(uint32 width, uint32 height, const char *title);

void setMousePosition(real64 x, real64 y);
void setMouseHidden(bool hidden);
void setMouseLocked(bool locked);

bool getVSYNCMode(void);
void switchVSYNCMode(void);
void setVSYNCMode(bool vsync);

bool getFullscreenMode(void);
void switchFullscreenMode(void);
void setFullscreenMode(bool fullscreen);

void getViewportSize(int32 *width, int32 *height);

void setWindowPosition(int32 xPosition, int32 yPosition);
void setWindowSize(int32 width, int32 height);

int32 closeWindow(void);
int32 freeWindow(struct GLFWwindow *window);