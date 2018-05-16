#pragma once
#include "defines.h"

int32 initRenderer(void);
int32 renderModel(const char *name, kmMat4 *world, kmMat4 *view, kmMat4 *projection);
