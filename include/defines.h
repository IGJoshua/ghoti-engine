#pragma once

#define internal static
#define persistent static
#define global static

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

#define VSYNC 0

#define ASSERTION_FAILED 1

#ifdef _DEBUG
#include <stdio.h>
#include <stdlib.h>
#define ASSERT(test) if (!(test))\
	{\
		printf("Assertion: %s failed in file %s on line %d\n", #test, __FILE__, __LINE__);\
		exit(ASSERTION_FAILED);\
	}
#else
#define ASSERT(test)
#endif
