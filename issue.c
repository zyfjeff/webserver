#include <fcntl.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "module.h"

static char* page_start = "<html><body><pre>";
static char* page_end = "</pre></body></html>";
static char* error_page = "<html><body><p>Error: Could not open /proc/issue. </p></body></html>";
static char* error_message = "Error reading /etc/issue";

void module_generate(int fd)
{
	int input_fd;
	struct stat file_info;
	int rval;
	input_fd = open("/etc/issue",O_RDONLY);
	if (input_fd == -1)
		system_error("open");
	rval = fstat(input_fd,&file_info);
	if (rval == -1) {
		write(fd,error_page,strlen(error_page));
	} else {
		int rval;
		off_t offset = 0;
		write(fd,page_start,strlen(page_start));
		rval = sendfile(fd,input_fd,&offset,file_info.st_size);
		if (rval == -1)
			write(fd,error_message,strlen(error_message));
		write(fd,page_end,strlen(page_end));
	}
	close(input_fd);
}
