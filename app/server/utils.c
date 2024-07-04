#include "../headers/utils.h"
#include <stdio.h>
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
                             int* max_desk,
                             const char* temp_path)
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
  handle_request(info, master_desk, sock, temp_path);
  return 0;
}

int
handle_request(Http_Info* info,
               fd_set* master,
               int connection_sock,
               const char* temp_path)
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
    if (index_response(connection_sock, RESPONSE_IF_CONNECTED))
      return -1;
  } else if (strncmp(path, "/echo/", strlen("/echo/")) == 0) {
    *end_path = '\0';
    if (echo_response(connection_sock, path + strlen("/echo/")))
      return -1;
  } else if (strncmp(path, "/user-agent", strlen("/user-agent")) == 0) {
    *end_path = ' ';
    if (user_agent_response(connection_sock, ++end_path))
      return -1;
  } else if (strncmp(path, "/files/", strlen("/files/")) == 0) {
    *(path - 1) = '\0';
    if (strcmp(recv_buf, "POST") == 0) {
      *(path - 1) = ' ';
      if (create_edit_file_response(
            connection_sock, temp_path, path + strlen("/files/"), recv_buf)) {
        return -1;
      }
    } else {
      *(path - 1) = ' ';
      if (file_response(connection_sock, temp_path, path + strlen("/files/")))
        return -1;
    }
  } else {
    if (not_found_response(connection_sock))
      return -1;
  }
  return 0;
}
char*
get_header_val(const char* header_name, char* buf)
{
  int match_counter = 0;
  for (int i = 0; i < strlen(buf); i++) {
    if (buf[i] == *header_name) {
      for (int j = i; j < strlen(header_name) + i; j++) {
        if (buf[j] == *(header_name + j - i))
          match_counter++;
        else
          break;
      }
      if (match_counter == strlen(header_name)) {
        return buf + i + match_counter +
               strlen(": "); // User-Agent: {our value}\r\n
      } else {
        match_counter = 0;
      }
    }
  }
  return NULL;
}

File_Info*
create_or_init_file(const char* temp_path, char* file_name, Query_Type type)
{
  File_Info* fli = (File_Info*)malloc(sizeof(*fli));
  char* specify_file_access = (char*)malloc(sizeof(char) * strlen("a+"));
  memset(specify_file_access, 0, strlen("a+"));
  if (type == GET)
    memcpy(specify_file_access, "r", strlen("r"));
  else
    memcpy(specify_file_access, "a+", strlen("a+"));
  memset(fli, 0, sizeof(*fli));
  memcpy(fli->complete_path, temp_path, strlen(temp_path));
  memcpy(fli->complete_path + strlen(temp_path), file_name, strlen(file_name));

  if ((fli->file_ptr = fopen(fli->complete_path, specify_file_access)) !=
      NULL) {
    fseek(fli->file_ptr, 0, SEEK_END);
    fli->raw_sz = ftell(fli->file_ptr);
    rewind(fli->file_ptr);
  }
  free(specify_file_access);
  return fli;
}

void
shutdown_http_server(Http_Info** init_http_ptr)
{
  close((*init_http_ptr)->master_socket);
  free((*init_http_ptr));

  printf("http server status:\t OFF\n");
}

Encoding
get_encode_type(char* buf)
{
  Encoding encoding_status = INVALID;
  Encode_Arr* encode_types = parse_encoding_arr(buf);
  if (!encode_types) {
    return encoding_status;
  }
  for (int i = 0; i < encode_types->sz; i++) {
    if (strncmp(encode_types->buf[i], "gzip", strlen("gzip")) == 0)
      encoding_status = GZIP;
  }
  free_encode_arr(encode_types);
  return encoding_status;
}

Encode_Arr*
parse_encoding_arr(char* buf)
{
  Encode_Arr* encode_types = (Encode_Arr*)malloc(sizeof(*encode_types));
  char* encoding_status = get_header_val("Accept-Encoding", buf);
  if (!encoding_status)
    return NULL;
  char* encoding_status_end_symbol = strchr(encoding_status, '\r');
  *encoding_status_end_symbol = '\0';
  char* buf_cpy = (char*)malloc(strlen(encoding_status) + sizeof('\0'));
  memset(buf_cpy, 0, strlen(encoding_status) + sizeof('\0'));
  memcpy(buf_cpy, encoding_status, strlen(encoding_status));
  char* divide_symbol_space = buf_cpy;
  encode_types->sz = 0;
  encode_types->buf = (char**)malloc(sizeof(char*) * (encode_types->sz + 1));
  while (divide_symbol_space) {
    if (!encode_types->sz)
      encode_types->buf[encode_types->sz] = buf_cpy;
    else
      encode_types->buf[encode_types->sz] =
        buf_cpy + (divide_symbol_space - buf_cpy);
    encode_types->sz++;
    divide_symbol_space = strchr(divide_symbol_space, ' ');
    if (divide_symbol_space != NULL) {
      *divide_symbol_space = '\0';
      ++divide_symbol_space;
      encode_types->buf =
        realloc(encode_types->buf, sizeof(char*) * (encode_types->sz + 1));
    }
  }

  *encoding_status_end_symbol = '\r';
  return encode_types;
}
void
free_encode_arr(Encode_Arr* arr)
{
  free(*(arr->buf));
  free(arr);
}
