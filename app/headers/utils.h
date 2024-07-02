#ifndef __UTILS_H__
#define __UTILS_H__

#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
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
#define HTTP_SERVER_MAX_LISTEN_CLIENTS 5

/* if init is correct should return 0 otherwise -1 */
int
init_http_server(Http_Info** init_http_ptr);
void
shutdown_http_server(Http_Info** init_http_ptr);
/* if handle is correct should return 0 otherwise -1 */
int
handle_new_client_connection(Http_Info* info, fd_set* master_desk, int* max_sz);
/* if handle is correct should return 0 otherwise -1 */
int
handle_request(Http_Info* info, fd_set* master_desk, int connection_sock);

#endif
