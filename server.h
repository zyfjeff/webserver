#ifndef _SERVER_H_
#define _SERVER_H_

#include <netinet/in.h>
#include <sys/types.h>
#include <stdint.h>

extern void server_run(struct in_addr local_addrsss,uint16_t port);

#endif //end of the _SERVER_H_
