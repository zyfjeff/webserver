#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "common.h"
#include "module.h"


static char* page_start = "<html><body><pre>";
static char* page_end = "</pre></body></html>";

void module_generate(int fd)
{
	int rval = 0;
	pid_t child_pid;
	write(fd,page_start,strlen(page_start));
	child_pid = fork();
	if (child_pid == 0) {
		char* argv[] = {"/bin/df","-h",NULL};
		rval = dup2(fd,STDOUT_FILENO);
		if (rval == -1)
			system_error("dup2");
		rval = dup2(fd,STDERR_FILENO);
		if (rval == -1)
			system_error("dup2");
		execv(argv[0],argv);
		system_error("execv");
	} else if (child_pid > 0) {
		rval = waitpid(child_pid,NULL,0);
		if (rval == -1)
			system_error("waitpid");
		else
			write(fd,page_end,strlen(page_end));
	}
}
