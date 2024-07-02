#include "../headers/utils.h"
#include <string.h>
#include <sys/socket.h>

const char* RESPONSE_IF_CONNECTED = "HTTP/1.1 200 OK\r\n\r\n";

int
init_http_server(Http_Info** init_http_ptr)
{
  *init_http_ptr = (Http_Info*)malloc(sizeof(Http_Info));
  int server_fd, client_addr_len;
  struct sockaddr_in client_addr;

  server_fd = (*init_http_ptr)->master_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    printf("socket():\t %d, %s", errno, strerror(errno));
    return (1);
  }

  /*Since the tester restarts your program quite often, setting SO_REUSEADDR
   ensures that we don't run into 'Address already in use' errors*/
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    printf("SO_REUSEADDR failed: %s \n", strerror(errno));
    return (1);
  }
  struct sockaddr_in serv_addr = { .sin_family = AF_INET,
                                   .sin_port = htons(HTTP_SERVER_PORT),
                                   .sin_addr = { htonl(INADDR_ANY) },
                                   .sin_zero = 0 };

  memcpy(&(*init_http_ptr)->settings, &serv_addr, sizeof(serv_addr));

  if (bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0) {
    printf("Bind failed: %s \n", strerror(errno));
    return (1);
  }

  if (listen(server_fd, HTTP_SERVER_MAX_LISTEN_CLIENTS) != 0) {
    printf("Listen failed: %s \n", strerror(errno));
    return (1);
  }

  (*init_http_ptr)->settings_len = sizeof(client_addr);

  return 0;
}

int
handle_new_client_connection(Http_Info* info,
                             fd_set* master_desk,
                             int* max_desk)
{
  int sock;
  if ((sock = accept(info->master_socket,
                     (struct sockaddr*)&info->settings,
                     (socklen_t*)&info->settings_len)) == -1) {
    printf("accept():\t%d,%s\n", errno, strerror(errno));
    return -1;
  }
  FD_SET(sock, master_desk);
  *max_desk = sock > *max_desk ? sock : *max_desk;
  handle_request(info, master_desk, sock);
  return 0;
}

int
handle_request(Http_Info* info, fd_set* master, int connection_sock)
{
  char recv_buf[4096] = { 0 };
  int recv_bytes = recv(connection_sock, recv_buf, sizeof(recv_buf), 0);
  if (recv_bytes <= 1) {
    printf("Client close the connection\n");
    FD_CLR(connection_sock, master);
    return 0;
  }
  char* path = strchr(recv_buf, ' ');
  char* end_path = strchr(++path, ' ');
  *end_path = '\0';
  if (strcmp(path, "/") == 0) {
    send(
      connection_sock, RESPONSE_IF_CONNECTED, strlen(RESPONSE_IF_CONNECTED), 0);
  } else {
    send(connection_sock,
         "HTTP/1.1 404 Not Found\r\n\r\n",
         strlen("HTTP/1.1 404 Not Found\r\n\r\n"),
         0);
  }
  return 0;
}

void
shutdown_http_server(Http_Info** init_http_ptr)
{
  close((*init_http_ptr)->master_socket);
  free((*init_http_ptr));

  printf("http server status:\t OFF\n");
}
