#pragma once

#define internal static

typedef unsigned long long int64;
typedef int int32;
typedef short int16;
typedef char int8;

typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef double real64;
typedef float real32;

typedef unsigned char bool;
#define true 1
#define false 0

#define UUID_LENGTH 63

typedef union uuid_t
{
	char string[UUID_LENGTH + 1];
	uint8 bytes[UUID_LENGTH + 1];
} UUID;

#define ASSERTION_FAILED 1

#ifdef _DEBUG
#include <stdio.h>
#include <stdlib.h>
#define ASSERT(test) if (!(test))							\
	{														\
		LOG(												\
			"Assertion: %s failed in file %s on line %d\n",	\
			#test,											\
			__FILE__,										\
			__LINE__);										\
		volatile int32* crash = 0;							\
		*crash = 0;											\
	}
#else
#define ASSERT(test) if (!(test))							\
	{														\
		volatile int32* crash = 0;							\
		*crash = 0;											\
	}
#endif

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define EXIT_THREAD(returnValue) pthread_exit(returnValue); \
								 return returnValue