#ifndef UNIFIED_H
#define UNIFIED_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<string>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <map>

extern std::map<int,struct sockaddr_in*> fd_addr_map_;

enum {MAXLINE=4096,NAME_MAX_SIZE=512,METHOD_SIZE=10};

void Error(const char*);
void Http_server(int connfd);

#endif
