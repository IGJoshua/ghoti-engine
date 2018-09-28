#pragma once
#include "defines.h"

#include <GL/glew.h>
#include <GL/glu.h>

#include <GLFW/glfw3.h>

int32 initInput(GLFWwindow *window);
void shutdownInput(void);

void inputHandleEvents(void);
void clearGUIInput(void);
void handleGUIInput(real64 dt);