#include "../headers/utils.h"
#include <stdio.h>
#include <string.h>

#define STR(val) #val

int
index_response(int sock_fd, const char* str)
{
  int sent_bytes = send(sock_fd, str, strlen(str), 0);
  if (sent_bytes == -1) {
    printf("send() error:\t %d,%s", errno, strerror(errno));
    return -1;
  }
  return 0;
}
int
not_found_response(int sock_fd)
{
  const char* str = "HTTP/1.1 404 Not Found\r\n\r\n";
  int sent_bytes = send(sock_fd, str, strlen(str), 0);
  if (sent_bytes == -1) {
    printf("send() error:\t %d,%s", errno, strerror(errno));
    return -1;
  }
  return 0;
}

int
echo_response(int sock_fd, char* value)
{
  char* http_chunk =
    "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: \r\n\r\n";
  char* http_response = (char*)malloc(strlen(http_chunk) + 1 + strlen(value));
  sprintf(http_response,
          "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
          "%lu\r\n\r\n%s",
          strlen(value),
          value);
  int sent_bytes = send(sock_fd, http_response, strlen(http_response), 0);
  free(http_response);
  if (sent_bytes == -1) {
    printf("send() error:\t %d,%s", errno, strerror(errno));
    return -1;
  }
  return 0;
}

int
user_agent_response(int sock_fd, char* value)
{
  char* user_agent_parse_val = get_header_val("User-Agent", value);
  if (user_agent_parse_val == NULL)
    return -1;
  char* http_chunk =
    "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: \r\n\r\n";
  char* http_response =
    (char*)malloc(strlen(http_chunk) + 1 + strlen(user_agent_parse_val) +
                  strlen(STR(user_agent_parse_val)));
  sprintf(http_response,
          "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
          "%lu\r\n\r\n%s",
          strlen(user_agent_parse_val),
          user_agent_parse_val);
  int sent_bytes = send(sock_fd, http_response, strlen(http_response), 0);
  free(http_response);
  if (sent_bytes == -1) {
    printf("send() error:\t %d,%s", errno, strerror(errno));
    return -1;
  }
  return 0;
}
int
file_response(int sock_fd, const char* temp_path, char* file_name)
{
  size_t path_len = strlen(temp_path) + strlen(file_name) + 1;
  char* complete_path = (char*)malloc(path_len);
  memset(complete_path, 0, path_len);
  memcpy(complete_path, temp_path, strlen(temp_path));
  memcpy(complete_path + strlen(temp_path), file_name, strlen(file_name));
  FILE* fd_ptr = fopen(complete_path, "r");
  if (fd_ptr == NULL) {
    not_found_response(sock_fd);
  }
  fseek(fd_ptr, 0, SEEK_END);
  size_t file_length = ftell(fd_ptr);
  printf("file_length is %zu\n", file_length);
  rewind(fd_ptr);
  char* http_chunk = "HTTP/1.1 200 OK\r\nContent-Type: "
                     "application/octet-stream\r\nContent-Length: \r\n\r\n";
  char* http_response = (char*)malloc(strlen(http_chunk) + 1 +
                                      strlen(STR(file_length)) + file_length);
  char* file_buf = calloc(file_length, sizeof(char));
  fread(file_buf, sizeof(char), file_length, fd_ptr);
  printf("file_buf is %s\n", file_buf);
  sprintf(http_response,
          "HTTP/1.1 200 OK\r\nContent-Type: "
          "application/octet-stream\r\nContent-Length: %zu\r\n\r\n%s",
          file_length,
          file_buf);
  int sent_bytes = send(sock_fd, http_response, strlen(http_response), 0);

  free(complete_path);
  free(file_buf);
  free(http_response);
  fclose(fd_ptr);

  if (sent_bytes == -1) {
    printf("send() error:\t %d,%s\n", errno, strerror(errno));
    return -1;
  }
  return 0;
}
