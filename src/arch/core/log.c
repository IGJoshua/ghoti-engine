#include "core/log.h"

#include <stdarg.h>
#include <malloc.h>
#include <string.h>

void logFunction(const char *format, ...)
{
	va_list args;
    va_start(args, format);

	char *message = malloc(strlen(format) + 4096);
	vsprintf(message, format, args);
    va_end(args);

	LOG("%s", message);

	free(message);
}