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
  /* convert clean /echo/{val} to query again */
  char* end_buf = strchr(value, '\0');
  *end_buf = ' ';

  Encoding encode_status = get_encode_type(value);
  char* parse_value_for_answer = strchr(value, ' ');
  char* http_chunk =
    "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: \r\n\r\n";

  char* http_response = NULL;
  *parse_value_for_answer = '\0';
  size_t http_response_sz = 0;
  switch (encode_status) {
    case GZIP: {

      char gzip_buffer[GZIP_BUFFER_SIZE] = { 0 };
      int compressed_length =
        compressToGzip(value, strlen(value), gzip_buffer, sizeof(gzip_buffer));
      http_response_sz += strlen(http_chunk) + sizeof('\0') + strlen(value) +
                          strlen("Content-Encoding: gzip\r\n") +
                          strlen(STR(compressed_length)) + compressed_length;
      http_response = (char*)malloc(http_response_sz);
      memset(http_response, 0, http_response_sz);
      sprintf(http_response,
              "HTTP/1.1 200 OK\r\nContent-Type: "
              "text/plain\r\nContent-Encoding:gzip\r\nContent-Length: "
              "%d\r\n\r\n",
              compressed_length);
      http_response_sz += compressed_length;
      memcpy(strchr(http_response, '\0'), gzip_buffer, compressed_length);
      break;
    }
    case INVALID: {
      printf("in invalid\n");
      http_response_sz = strlen(http_chunk) + sizeof('\0') + strlen(value);
      http_response = (char*)malloc(http_response_sz);
      sprintf(http_response,
              "HTTP/1.1 200 OK\r\nContent-Type: "
              "text/plain\r\nContent-Length: "
              "%lu\r\n\r\n%s",
              strlen(value),
              value);

      break;
    }
    default:
      break;
  }
  printf("http_response is %s\n", http_response);
  int sent_bytes = send(sock_fd, http_response, http_response_sz, 0);
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
  Encoding encode_status = get_encode_type(value);
  char* user_agent_val = get_header_val("User-Agent", value);
  char* http_response = NULL;
  char* parse_user_agent_val = strchr(user_agent_val, '\r');
  *parse_user_agent_val = '\0';
  if (user_agent_val == NULL)
    return -1;
  char* http_chunk =
    "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: \r\n\r\n";
  switch (encode_status) {
    case GZIP: {
      http_response = (char*)malloc(
        strlen(http_chunk) + sizeof('\0') + strlen(user_agent_val) +
        strlen(STR(user_agent_val)) + strlen("Content-Encoding: gzip\r\n"));
      sprintf(http_response,
              "HTTP/1.1 200 OK\r\nContent-Type: "
              "text/plain\r\nContent-Encoding: gzip\r\nContent-Length: "
              "%lu\r\n\r\n%s",
              strlen(user_agent_val),
              user_agent_val);
      break;
    }
    case INVALID: {
      http_response =
        (char*)malloc(strlen(http_chunk) + sizeof('\0') +
                      strlen(user_agent_val) + strlen(STR(user_agent_val)));
      sprintf(http_response,
              "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
              "%lu\r\n\r\n%s",
              strlen(user_agent_val),
              user_agent_val);
      break;
    }
    default:
      break;
  }
  printf("http_response is %s\n", http_response);
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
  /* convert clean file_name to query again*/
  char* end_buf = strchr(file_name, '\0');
  *end_buf = ' ';

  Encoding encode_status =
    get_encode_type(file_name); /* file_name because its ptr on recv_buffer,
                                 *maybe its shitty anyways....*/
  *end_buf = '\0';

  size_t buf_sz = 1024;
  Query_Type type = GET;
  int read_bytes = 0;
  File_Info* fle = create_or_init_file(temp_path, file_name, type);
  if (fle->file_ptr == NULL) {
    not_found_response(sock_fd);
  } else {
    char* http_chunk = "HTTP/1.1 200 OK\r\nContent-Type: "
                       "application/octet-stream\r\nContent-Length: \r\n\r\n";
    char* http_response = NULL;
    char* file_buf = calloc(buf_sz, sizeof(char));
    switch (encode_status) {
      case GZIP: {
        http_response = (char*)malloc(strlen(http_chunk) + sizeof('\0') +
                                      strlen(STR(fle->raw_sz)) +
                                      strlen("Content-Encoding: gzip\r\n"));
        sprintf(http_response,
                "HTTP/1.1 200 OK\r\nContent-Type: "
                "application/octet-stream\r\nContent-Encoding: "
                "gzip\r\nContent-Length: %zu\r\n\r\n",
                fle->raw_sz);
        break;
      }
      case INVALID: {
        http_response = (char*)malloc(strlen(http_chunk) + sizeof('\0') +
                                      strlen(STR(fle->raw_sz)));
        sprintf(http_response,
                "HTTP/1.1 200 OK\r\nContent-Type: "
                "application/octet-stream\r\nContent-Length: %zu\r\n\r\n",
                fle->raw_sz);
        break;
      }
      default:
        break;
    }
    int sent_bytes = send(sock_fd, http_response, strlen(http_response), 0);
    int index = 0;
    memset(file_buf, 0, buf_sz);
    while ((read_bytes = fread(file_buf, sizeof(char), buf_sz, fle->file_ptr)) >
           0) {
      sent_bytes = send(sock_fd, file_buf, read_bytes, 0);
      if (sent_bytes == -1) {
        printf("send() error:\t %d,%s\n", errno, strerror(errno));
        return -1;
      }
      memset(file_buf, 0, read_bytes);
      ++index;
    }
    fclose(fle->file_ptr);
    free(fle);
    free(file_buf);
    free(http_response);
  }
  return 0;
}

int
create_edit_file_response(int sock_fd,
                          const char* temp_path,
                          char* file_name,
                          char* raw_query)
{
  size_t buf_sz = 1024;
  char *body = NULL, *body_cpy = NULL, *body_sz = NULL;
  Query_Type type = POST;
  File_Info* fle = create_or_init_file(temp_path, file_name, type);
  if (fle->file_ptr == NULL) {
    printf("fopen() error, %d,%s\n", errno, strerror(errno));
    return -1;
  }
  char* end = strchr(raw_query, '\0');
  *end = ' ';
  body_cpy = strchr(raw_query, '\n');
  while ((body_cpy = strchr(++body_cpy, '\n')) != NULL)
    body = body_cpy;
  ++body;
  body_sz = get_header_val("Content-Length", raw_query);
  char* end_body = strchr(body_sz, '\r');
  *end_body = '\0';
  int write_bytes = fwrite(body, sizeof(char), atoi(body_sz), fle->file_ptr);
  if (write_bytes == 0) {
    printf("fwrite() error,%d %s\n", errno, strerror(errno));
    return -1;
  }
  fclose(fle->file_ptr);
  free(fle);
  int sent_bytes = send(sock_fd,
                        "HTTP/1.1 201 Created\r\n\r\n",
                        strlen("HTTP/1.1 201 Created\r\n\r\n"),
                        0);
  if (sent_bytes == -1) {
    printf("send() error:\t %d,%s\n", errno, strerror(errno));
    return -1;
  }
  return 0;
}
