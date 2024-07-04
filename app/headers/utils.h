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

static char* PATH;

typedef struct Http_Info
{
  int master_socket;
  struct sockaddr_in settings;
  socklen_t settings_len;
} Http_Info;

typedef struct File_Info
{
  size_t raw_sz;
  FILE* file_ptr;
  char complete_path[256];
} File_Info;

typedef enum Query_Type
{
  GET,
  POST
} Query_Type;

typedef enum Encoding
{
  INVALID,
  GZIP
} Encoding;

#define HTTP_SERVER_PORT 4221
#define HTTP_SERVER_MAX_LISTEN_CLIENTS 5

/* if init is correct should return 0 otherwise -1 */
int
init_http_server(Http_Info** init_http_ptr);
void
shutdown_http_server(Http_Info** init_http_ptr);
/* if handle is correct should return 0 otherwise -1 */
int
handle_new_client_connection(Http_Info* info,
                             fd_set* master_desk,
                             int* max_sz,
                             const char* temp_path);
/* if handle is correct should return 0 otherwise -1 */
int
handle_request(Http_Info* info,
               fd_set* master_desk,
               int connection_sock,
               const char* temp_path);

/* if error should return -1 otherwise return 0  */
int
index_response(int sock_fd, const char* str);

/* if error should return -1 otherwise return 0 */
int
not_found_response(int sock_fd);

/* if error should return -1 otherwise return 0 */
int
echo_response(int sock_fd, char* value);

/* if error should return -1 otherwise return 0 */
int
user_agent_response(int sock_fd, char* value);
/*
 * get clean value with \0
 * if error should return NULL
 * */
char*
get_header_val(const char* header_name, char* buf);

/* if error shoult return -1 otherwise 0*/
int
file_response(int sock_fd, const char* temp_path, char* file_name);

/* if error should return -1 otherwise 0*/
int
create_edit_file_response(int sock_fd,
                          const char* temp_path,
                          char* file_name,
                          char* raw_query);

/* if error FILE_INFO->file_ptr == NULL */
File_Info*
create_or_init_file(const char* temp_path, char* file_name, Query_Type type);

/* if don't have any query about encoding or haven't incoding handle return
 * INVALID */
Encoding
get_encode_type(char* buf);
#endif
