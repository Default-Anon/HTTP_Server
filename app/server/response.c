#include "../headers/utils.h"
#include <string.h>

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
  printf("user_agent_parse_val is %s\n", user_agent_parse_val);
  char* http_chunk =
    "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: \r\n\r\n";
  char* http_response =
    (char*)malloc(strlen(http_chunk) + 1 + strlen(user_agent_parse_val));
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
