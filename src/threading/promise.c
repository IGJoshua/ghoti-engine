#include "threading/promise.h"

#include "threading/threading_types.h"

#include <malloc.h>

Promise *pCreate()
{
	Promise *ret = malloc(sizeof(Promise));

	pthread_mutex_init(&ret->mut, NULL);
	pthread_cond_init(&ret->cond, NULL);

	return ret;
}

Promise *pReturn(void *pData)
{
	Promise *ret = malloc(sizeof(Promise));

	ret->pData = pData;
	ret->isDone = 1;

	pthread_mutex_init(&ret->mut, NULL);
	pthread_cond_init(&ret->cond, NULL);

	return ret;
}

Promise *pBind(Promise **p, Promise *(*fn)(void *pData))
{
	pthread_mutex_lock(&(*p)->mut);

	pthread_cond_wait(&(*p)->cond, &(*p)->mut);

	Promise *ret = fn((*p)->pData);

	pthread_mutex_unlock(&(*p)->mut);

	freePromise(p);

	return ret;
}

void pComplete(Promise *p, void *pData)
{
	pthread_mutex_lock(&p->mut);

	p->pData = pData;
	p->isDone = 1;

	pthread_cond_signal(&p->cond);

	pthread_mutex_unlock(&p->mut);
}

void freePromise(Promise **p)
{
	if (*p)
	{
		pthread_mutex_destroy(&(*p)->mut);
		pthread_cond_destroy(&(*p)->cond);
		free(*p);
	}

	*p = NULL;
}
