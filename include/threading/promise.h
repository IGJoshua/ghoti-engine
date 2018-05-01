#pragma once
#include "defines.h"

#include "threading_types.h"

Promise *pCreate();
Promise *pReturn(void *pData);
Promise *pBind(Promise **p, Promise *(*fn)(void *pData));

void pComplete(Promise *p, void *pData);

void freePromise(Promise **p);
