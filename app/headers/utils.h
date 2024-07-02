#ifndef __UTILS_H__
#define __UTILS_H__

#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

typedef struct Http_Info
{
  int master_socket;
  struct sockaddr_in settings;
  socklen_t settings_len;
} Http_Info;

#define HTTP_SERVER_PORT 4221
#define HTTP_SERVER_MAX_LISTEN_CLIENTS 1

/* if correct init should return 0 otherwise 1 */
int init_http_server (Http_Info **init_http_ptr);
void shutdown_http_server (Http_Info **init_http_ptr);

#endif
