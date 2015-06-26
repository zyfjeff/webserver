#ifndef _COMMON_H_
#define _COMMON_H_

#include <sys/types.h>

/* symbols defined common.c */

/* THe name of this program */

extern const char* program_name;

/* if  nonzero,print verbose message */

extern int verbose;

/* lije malloc,except aborts the program if allocation fails */
extern void *xmalloc(size_t);

extern void* xrealloc(void *ptr,size_t size);
/* like strdup,except aborts the program if allocatiom fails */
extern char* xstrdup(const char *s);

/* print an error message for a failed call OPERATIOON,usng the value
	of errno, and end the program */

extern void system_error(const char* operation);

/* print an error message for failure involving CAUSE,
	include a description MESSAGE,and end the program */
extern void error(const char* cause,const char* message);

extern char* get_self_executable_directory();

#endif //end of _COMMON_H_
