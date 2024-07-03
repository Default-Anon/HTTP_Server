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
  printf("value is %s\n", value);
  char* http_chunk =
    "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: \r\n\r\n";
  char* http_response = (char*)malloc(strlen(http_chunk) + 1 + strlen(value));
  sprintf(http_response,
          "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
          "%lu\r\n\r\n%s",
          strlen(value),
          value);
  printf("sent_buf is %s\n", http_response);
  int sent_bytes = send(sock_fd, http_response, strlen(http_response), 0);
  free(http_response);
  if (sent_bytes == -1) {
    printf("send() error:\t %d,%s", errno, strerror(errno));
    return -1;
  }
  return 0;
}
