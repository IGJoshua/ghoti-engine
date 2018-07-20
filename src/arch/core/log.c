#include "core/log.h"

internal FILE *file;

void initLog(void)
{
	file = fopen(LOG_FILE_NAME, "w");
}

FILE* getLogFile(void)
{
	return file;
}

void shutdownLog(void)
{
	fclose(file);
}