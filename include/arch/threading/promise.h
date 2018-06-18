#pragma once
#include "defines.h"

#include "threading_types.h"

Promise *pCreate(void);
void freePromise(Promise **p);

Promise *pReturn(void *pData);
Promise *pBind(Promise **p, Promise *(*fn)(void *pData));

void pComplete(Promise *p, void *pData);
