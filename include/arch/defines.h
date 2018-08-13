#pragma once

#define internal static
#define persistent static

typedef long int64;
typedef int int32;
typedef short int16;
typedef char int8;

typedef unsigned long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef double real64;
typedef float real32;

typedef enum { false, true } bool;

#define UUID_LENGTH 63

typedef union uuid_t
{
	char string[UUID_LENGTH + 1];
	uint8 bytes[UUID_LENGTH + 1];
} UUID;

#define VSYNC 1

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