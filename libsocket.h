#ifndef _LIB_LIBSOCKET_H
#define _LIB_LIBSOCKET_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

#define ERR_EXIT(msg) do{perror(msg);exit(EXIT_FAILURE);}while(0)

typedef enum {SUCCESS=0,FAIL=-1}STATUS;

bool settimeout(int sockfd,int timeout);
int ip_addr(char *p_str);
void create_bro_addr(int port,struct sockaddr_in *p_addrto);
int create_bro_socket(int port);
int create_tcp_socket(unsigned int port,unsigned int backlog);
int create_udp_socket();
bool setbroadcast(int sockfd);
bool setnonblocking(int sockfd);
bool setfdreuseaddr(int sockfd);
ssize_t readn(int sockfd,void *p_ptr,size_t n);
ssize_t writen(int sockfd,void *p_ptr,size_t n);
int read_timeout(int fd,int timeout);
int write_timeout(int fd,int timeout);
int accept_timeout(int fd,struct sockaddr_in *peeraddr,int timeout);
int connect_timeout(int fd,struct sockaddr_in *addr,int timeout);
#endif // _LIB_LIBSOCKET_H
