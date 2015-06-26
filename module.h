#ifndef _MODULE_H_
#define _MODULE_H_


/* An instance of a loaded server module */
struct server_module {
	/* The shared library handle corresponding to the loaded module */
	void *handle;
	/* A name describing the module */
	const char* name;
	/* The function that generates the HTML results for this module */
	void (*generate_function)(int);
};

extern char *module_dir;
extern struct server_module* module_open(const char *module_path);
extern void module_close(struct server_module* module);


#endif //end of _MODULE_H_
