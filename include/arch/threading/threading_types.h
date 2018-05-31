#pragma once
#include "defines.h"

#include <pthread.h>

typedef struct promise_t
{
	void *pData;
	int32 isDone;
	pthread_cond_t cond;
	pthread_mutex_t mut;
} Promise;
