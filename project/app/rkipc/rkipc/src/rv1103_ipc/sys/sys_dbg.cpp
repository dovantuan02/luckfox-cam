#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "ak.h"

#include "sys_dbg.h"

void sys_dbg_fatal(const char *s, uint8_t c) {
	printf("[SYS_UNIX] FATAL: %s \t 0x%02X\n", s, c);
	exit(EXIT_FAILURE);
}

char *sys_dbg_get_time(void) {
	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	static char return_val[64];
	strftime(return_val, sizeof(return_val), "%Y-%m-%d %H:%M:%S", timeinfo);

	struct timeval te;
	gettimeofday(&te, NULL);
	int millis = te.tv_sec * 1000 + te.tv_usec / 1000;

	char buf[32];
	sprintf(buf, " [%010d]", millis);
	strcat(return_val, buf);

	return (char *)return_val;
}
