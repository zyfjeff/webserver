#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>

#include "common.h"
#include "module.h"


static int get_uid_gid(pid_t pid,uid_t *uid,gid_t *gid)
{
	char dir_name[64];
	struct stat dir_info;
	int rval;
	snprintf(dir_name,sizeof(dir_name),"/proc/%d",(int)pid);
	rval = stat(dir_name,&dir_info);
	if (rval != 0) {
		int saveerror = errno;
		switch(saveerror) {
		case EACCES:
			fprintf(stderr,"EACCES\n");
			break;
		case EBADF:
			fprintf(stderr,"EBADF\n");
			break;
		case EFAULT:
			fprintf(stderr,"EFAULT\n");
			break;
		case ELOOP:
			fprintf(stderr,"ELOOP\n");
			break;
		case ENAMETOOLONG:
			fprintf(stderr,"ENAMETOOLONG\n");
			break;
		case ENOENT:
			fprintf(stderr,"ENOENT\n");
			break;
		case ENOMEM:
			fprintf(stderr,"ENOMEM\n");
			break;
		case ENOTDIR:
			fprintf(stderr,"ENOTDIR\n");
			break;
		case EOVERFLOW:
			fprintf(stderr,"EOVERFLOW\n");
			break;
		}
		perror("stat:");		
		fprintf(stderr,"stat error\n");
		return 1;
	}
	assert(S_ISDIR(dir_info.st_mode));
	*uid = dir_info.st_uid;
	*gid = dir_info.st_gid;
	return 0;
}

static char* get_user_name(uid_t uid)
{
	struct passwd* entry;
	entry = getpwuid(uid);
	if (entry == NULL)
		system_error("getpwuid");
	return xstrdup(entry->pw_name);
}

static char* get_group_name(gid_t gid)
{
	struct group* entry;
	entry = getgrgid(gid);
	if (entry == NULL)
		system_error("getgrgid");
	return xstrdup(entry->gr_name);
}

static char* get_program_name(pid_t pid)
{
	char file_name[64];
	char status_info[256];
	int fd;
	int rval;
	char *open_paren;
	char *close_paren;
	char *result;
	
	snprintf(file_name,sizeof(file_name),"/proc/%d/stat",(int)pid);
//	printf("filename:%s\n",file_name);
	fd = open(file_name,O_RDONLY);
	if (fd == -1)
		return NULL;
	rval = read(fd,status_info,sizeof(status_info)-1);
	close(fd);
	if (rval <= 0)
		return NULL;
	status_info[rval] = '\0';
	open_paren = strchr(status_info,'(');
	close_paren = strchr(status_info,')');
	if (open_paren == NULL || close_paren == NULL || close_paren < open_paren)
		return NULL;
	result = (char*)xmalloc(close_paren - open_paren);
	strncpy(result,open_paren+1,close_paren - open_paren -1);
	result[close_paren - open_paren - 1] = '\0';
	return result;
}

static int get_rss(pid_t pid)
{
	char file_name[64];
	char mem_info[128];
	int fd;
	int rval;
	int rss;
	
	snprintf(file_name,sizeof(file_name),"/proc/%d/statm",(int)pid);
	fd = open(file_name,O_RDONLY);
	if (fd == -1)
		return -1;
	rval = read(fd,mem_info,sizeof(mem_info) - 1);
	close(fd);
	if (rval <= 0)
		return 0;
	mem_info[rval] = '\0';
	rval = sscanf(mem_info,"%*d %d",&rss);
	if (rval != 1)
		return -1;
	return rss * getpagesize()/1024;
}

static char* format_process_info(pid_t pid)
{
	int rval;
	uid_t uid;
	gid_t gid;
	char* user_name;
	char* group_name;
	int rss;
	char* program_name;
//	size_t result_length;
	char* result;
	rval = get_uid_gid(pid,&uid,&gid);
	if(rval != 0) {
		fprintf(stderr,"get_uid_gid error\n");
		return NULL;
	}
	rss = get_rss(pid);
	if (rss == -1) {
		fprintf(stderr,"get_rss error\n");
		return NULL;
	}
	program_name = get_program_name(pid);
	if (program_name == NULL) {
		fprintf(stderr,"get_program_name error\n");
		return NULL;
	}
	user_name = get_user_name(uid);
	group_name = get_group_name(gid);
	
//	result_length = strlen(program_name);
	result = (char*)xmalloc(512);
	sprintf(result,
	      "<tr><td align=\"right\">%d</td><td><tt>%s</tt></td><td>%s</td><td>%s</td><td align=\"right\">%d</td></tr>",
	      (int)pid,program_name,user_name,group_name,rss);
	printf("%s\n",result);
	free(program_name);
	free(user_name);
	free(group_name);
	return result;
}

static char* page_start = "<html><body><table cellpadding=\"4\" cellspacing=\"0\" border=\"1\">\
<thead><tr><th>PID</th><th>Program</th><th>User</th><th>Group</th><th>RSS&nbsp;(KB)<th></tr></thead><tbody>";

static char* page_end = "</tbody></table></body></html>";

void module_generate(int fd)
{
	size_t i;
	char tmp_buf[256];
	DIR *proc_listing;
	size_t vec_length = 0;
	size_t vec_size = 16;
	struct iovec* vec = (struct iovec*)xmalloc(vec_size * sizeof(struct iovec));
	vec[vec_length].iov_base = page_start;
	vec[vec_length].iov_len = strlen(page_start);
	++vec_length;
	
	proc_listing = opendir("/proc");
	if (proc_listing == NULL)
		system_error("opendir");
	while(1) {
		struct dirent* proc_entry;
		const char* name;
		pid_t pid;
		char* process_info;
		proc_entry = readdir(proc_listing);
		if (proc_entry == NULL)
			break;
		name = proc_entry->d_name;
		printf("name:%s\n",name);
		if (strspn(name,"0123456789") != strlen(name))
			continue;
		sprintf(tmp_buf,"/proc/%s",name);
		if(access(tmp_buf,F_OK) == -1)
			continue;
		pid = (pid_t)atoi(name);
		process_info = format_process_info(pid);
//		printf("process_info:%s\n",process_info);
		if(process_info == NULL) {
			process_info = (char*)xmalloc(128);
			strcpy(process_info,"<tr><td colspan=\"5\">ERROR</td></tr>");
			fprintf(stderr,"error process_info\n");
		}
		if(vec_length == (vec_size -1)) {
			vec_size *= 2;
			vec = xrealloc(vec,vec_size * sizeof(struct iovec));
		}
		vec[vec_length].iov_base = process_info;
		vec[vec_length].iov_len = strlen(process_info);
		++vec_length;
	}
	closedir(proc_listing);
	vec[vec_length].iov_base = page_end;
	vec[vec_length].iov_len = strlen(page_end);
	++vec_length;
	writev(fd,vec,vec_length);
	for(i =1;i < vec_length-1;++i)
		free(vec[i].iov_base);
	free(vec);
}
