// Copyright(c) Beijing iAchieve S&T Co.Ltd. ALL rights reserved.
// Author: ZhangYiFei<zyfforlinux@163.com>
// Created: 2015-03-23
// Description: socket lib

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <poll.h>
#include <time.h>
#include <sys/time.h>
#include "libsocket.h"

int ip_addr(char *p_str)
{
        int inet_sock;
        struct ifreq ifr;
        inet_sock = socket(AF_INET, SOCK_DGRAM, 0); 
        strncpy(ifr.ifr_name, "eth0",strlen("eth0")+1);
        //SIOCGIFADDR标志代表获取接口地址
        if (ioctl(inet_sock, SIOCGIFADDR, &ifr) <  0) {
                perror("ioctl");
                p_str = NULL;
                return -1;
        }   
        char *p_ipstr = inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr);
        strncpy(p_str,p_ipstr,strlen(p_ipstr)+1);
        return 0;
}

/**
 *desc: craete a tcp socket,and bind the specify port and INADDR_ANY addr
 *input: port port number
 *      backlog connect queue size
 *output: socket fd
 *
 */
int create_tcp_socket(unsigned int port,unsigned int backlog)
{
	struct sockaddr_in addr;
	int sockfd = socket(PF_INET,SOCK_STREAM,0);
	if (sockfd == -1)
		ERR_EXIT("CreateTcpSocket:Create Socket Failure");
    setfdreuseaddr(sockfd);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	
	int ret = bind(sockfd,(struct sockaddr*)&addr,sizeof(addr));
	if (ret != 0)
		ERR_EXIT("CreateTcpSocket:Bind Socket Failure");
	ret = listen(sockfd,backlog);
	if (ret != 0)
		ERR_EXIT("CreateTcpSocket:Listen Socket Failure");
	return sockfd;
	
}

/**
 *desc: craete a broadcast sockaddr_in struct
 *input: addrto: a sockaddr_in point
 *output: None
 */
void create_bro_addr(int port,struct sockaddr_in *addrto)
{
    bzero(addrto, sizeof(struct sockaddr_in));
    addrto->sin_family=AF_INET;
    addrto->sin_addr.s_addr=htonl(INADDR_BROADCAST);
    addrto->sin_port=htons(port);
}

/**
 *desc: craete a broadcast socket
 *input: port: port to bind
 *output: socket fd to craete
 */
int create_bro_socket(int port)
{
    int sockfd = -1; 
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
    {   
        printf("soctet error\n");
        return -1; 
    }   
    if (setbroadcast(sockfd) == FAIL)
    return -1; 
 
    return sockfd;   
}

/**
 *desc: craete udp socket
 *input: port: udp bind port
 *output: udp socket fd
 */
int create_udp_socket(int port)
{
    int ret = 0;
    int sockfd = 0;
    struct sockaddr_in addr;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd <= 0)
    {
	ERR_EXIT("CreateUdpSocket socket create error:");
    }
    setfdreuseaddr(sockfd);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    if(ret == -1)
    {
        ERR_EXIT("CreateUdpSocket bind error:");
    }
    return sockfd;
}

/**
 *desc: 设置udp的套接字超时时间，进行阻塞读
 *input: sockfd udp的套接字描述
 *      timeout socket读的超时时间
 *output: 设置成功或者失败，具体参见FAIL/SUCCESS宏
 *
 */
bool settimeout(int sockfd,int timeout)
{
    struct timeval tv_out;
    tv_out.tv_sec = timeout;
    tv_out.tv_usec = 0;
    int ret = setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv_out, sizeof(tv_out));
    if (ret != 0)
        return FAIL;
    else
        return SUCCESS;
}


/**
 *desc: setup the socket nonblocking
 *input: sockfd: socket fd
 *output: setup success oi setup failure
 */
bool setnonblocking(int sockfd)
{
	int oldflag = fcntl(sockfd,F_GETFL);
	int newflag = oldflag|O_NONBLOCK;	
	int ret = fcntl(sockfd,F_SETFL,newflag);
	if (ret != 0)
		return false;
	else
		return true;
}

/**
 *desc: setup address reuse 
 *input: sockfd: socket fd
 *output: setup success or setup failure
 */
bool setfdreuseaddr(int sockfd)
{
	int value = 1;
	int ret = setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value));
	if (ret != 0)
		return false;
	else
		return true;	
}

/**
 *desc: setup socket fd for the broadcast 
 *input: sockfd: socket fd
 *output: setup success or failure
 */
bool setbroadcast(int sockfd)
{
    int value = 1;
    int ret = setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,
          (char *)&value,sizeof(value));
    if (ret != 0)
        return false;
    else
        return true;

}

/**
 *desc: make sure read n bytes from fd
 *input: sockfd: socket fd
 *      p_ptr: read buf point
 *      n: read buf size
 *output: actual read size
 */
ssize_t readn(int sockfd,void *p_ptr,size_t n)
{
	size_t nleft = n;
	size_t nread;
	char *p_buf = (char*)p_ptr;
	while(nleft > 0)
	{
		nread = recv(sockfd,p_buf,nleft,0);
		if (nread > 0) {
			nleft = nleft - nread;
			p_buf = p_buf + nread;
		} else if (nread == 0) {
			return n - nleft;	
		} else if (nread < 0) {
			if (errno == EINTR)
				continue;
			else
				return -1;
		}
	}
	return n-nleft;
}

/** 
 *desc: make sure write n bytes to fd
 *input: sockfd: socket fd 
 *      p_ptr: want to send buf
 *      n: want to send size
 *output: actual write size
 */

ssize_t writen(int sockfd,void *p_ptr,size_t n)
{
	size_t nleft = n;
	size_t nwrite;
	char *p_buf = (char *)p_ptr;
	while(nleft > 0)
	{
		nwrite = send(sockfd,p_buf,nleft,0);
		if (nwrite > 0) {
			nleft = nleft - nwrite;
			p_buf = p_buf + nwrite;
		} else if (nwrite == 0) {
			return n - nleft;
		} else {
			if (errno == EINTR)
				continue;
			else
				return -1;
		}
	}
	return n - nleft;
}

ssize_t recv_peek(int sockfd,void *buf,size_t len)
{
    while(1)
    {   
        int ret = recv(sockfd,buf,len,MSG_PEEK);
        if (ret == -1 && errno == EINTR)
            continue;
        return ret;
    }   
}


ssize_t readline(int sockfd,void *buf,size_t maxline)
{
    int ret;
    int nread;
    char *bufp = (char*)buf;
    int nleft = maxline;
    while(1)
    {   
        ret = recv_peek(sockfd,bufp,nleft);//一次性嗅探bufp个数据
        if (ret < 0) //发送错误
            return ret;
        else if (ret == 0)
            return ret;
        nread = ret;
        int i;
        for(i = 0;i < nread;i++) //查找是否有\n
        {
            if (bufp[i] == '\n')  //读取i+1个数据
            {
                ret = readn(sockfd,bufp,i+1);
                if (ret != i+1)
                    return -1; //发生错误
                return ret;
            }
        }
        if (nread > nleft) //读取错误
            return -1;
        nleft -= nread;  //减去读到的元素
        ret = readn(sockfd,bufp,nread); //把数据取走
        if (ret <= 0)
            return ret;
        bufp += ret;
    }
    return -1;
}

/*
 *desc: read with timeout
 *fd: file description
 *timeout: timeout value
 *return:
 *      -1: error could't to read
 *      0: could read
 */
int read_timeout(int fd,int timeout)
{

    setnonblocking(fd); //setup for nonblocking
    int ret;
    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    do{
        ret = poll(fds,1,timeout);
    }while(ret < 0 && errno == EINTR); //signale
    if (ret == 0) {   //timeout error
        errno = ETIMEDOUT;
        return -1;
    }
    if (ret == 1)  //success
        return 0;
    return -1;
}


/*
 *desc: write with timeout
 *fd: file description
 *timeout: timeout value
 *return:
 *      -1: error could't to write
 *      0: could write
 */
int write_timeout(int fd,int timeout)
{
    setnonblocking(fd);
    int ret;
    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLOUT;
    fds[0].revents = 0;
    do{
        ret = poll(fds,1,timeout);
    }while(ret < 0 && errno == EINTR); //signale
    if (ret == 0) {   //timeout error
        errno = ETIMEDOUT;
        return -1;
    }
    if (ret == 1)  //success
        return 0;
    return -1;
}

/*
 *desc: accept with timeout
 *fd: file description
 *peeraddr: peer ip address
 *timeout: timeout value
 *return:
 *      >0: perr fd
 *      -1: accept failure
 */

int accept_timeout(int fd,struct sockaddr_in *peeraddr,int timeout)
{
    printf("starting to accept\n");
    int ret = 0;
    setnonblocking(fd); 
    socklen_t len = sizeof(struct sockaddr_in);
    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
    if (timeout > 0 || timeout == -1) {
        do{
            ret = poll(fds,1,timeout);
        }while(ret < 0 && errno == EINTR);
        if (ret == 0) {
            errno = ETIMEDOUT;
            return -1;
        } else if (ret == -1)
            return -1;
    }
    if (peeraddr != NULL)
        ret = accept(fd,(struct sockaddr*)peeraddr,&len);
    else
        ret = accept(fd,NULL,NULL);
    if (ret == -1)
        ERR_EXIT("accept");
    return ret;
}


/*
 *desc: connect with timeout
 *fd: file description
 *peeraddr: connect to ip address
 *timeout: timeout value
 *return:
 *      0: connect success
 *      -1: connect failure
 */

int connect_timeout(int fd,struct sockaddr_in *addr,int timeout)
{
    int ret = 0;
    setnonblocking(fd); //setup for nonblocking
    socklen_t len = sizeof(struct sockaddr_in);
    struct pollfd fds[1];
    fds[0].fd = fd;
    fds[0].events = POLLOUT;
    fds[0].revents = 0;
    ret = connect(fd,(struct sockaddr*)addr,len);
    if (ret < 0 && errno == EINPROGRESS)
    {
        do{
            ret = poll(fds,1,timeout);
        }while(ret < 0 && errno == EINTR);
        if (ret == 0) {
            errno = ETIMEDOUT;
            return -1;
        } else if (ret < 0)
            return -1;
        else if (ret == 1) {
         //两种可能，一种就是产生了错误，另外一种才是连接建立成功    
            int err;
            socklen_t socklen = sizeof(err);
            int sockoptret = getsockopt(fd,SOL_SOCKET,SO_ERROR,&err,&socklen);
            if (sockoptret == -1){
                return -1;
            }
            if (err == 0)
                return 0;
            else {
                errno = err;
                return -1;
            }
        }
    }
    return ret;
}
   
