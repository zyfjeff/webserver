#include <assert.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "common.h"
#include "module.h"

static char* page_template = 
"<html><head><meta http-equiv=\"refresh\" content=\"5\"></head><body><p>THe current time is %s.</p></body></html>";

void module_generate(int fd)
{
	struct timeval tv;
	struct tm* ptm;
	char time_string[40];
	FILE *fp;
	gettimeofday(&tv,NULL);
	ptm = localtime(&tv.tv_sec);
	strftime(time_string,sizeof(time_string),"%H:%M:%S",ptm);
	fp = fdopen(fd,"w");
	assert(fp != NULL);
	fprintf(fp,page_template,time_string);
	fflush(fp);
}

