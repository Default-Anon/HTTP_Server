#include "../headers/utils.h"

int
init_http_server (Http_Info **init_http_ptr)
{
  *init_http_ptr = (Http_Info *)malloc (sizeof (Http_Info));
  int server_fd, client_addr_len;
  struct sockaddr_in client_addr;

  server_fd = (*init_http_ptr)->master_socket
      = socket (AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1)
    {
      printf ("socket():\t %d, %s", errno, strerror (errno));
      return (1);
    }

  /*Since the tester restarts your program quite often, setting SO_REUSEADDR
   ensures that we don't run into 'Address already in use' errors*/
  int reuse = 1;
  if (setsockopt (server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof (reuse))
      < 0)
    {
      printf ("SO_REUSEADDR failed: %s \n", strerror (errno));
      return (1);
    }
  struct sockaddr_in serv_addr = { .sin_family = AF_INET,
                                   .sin_port = htons (HTTP_SERVER_PORT),
                                   .sin_addr = { htonl (INADDR_ANY) },
                                   .sin_zero = 0 };
  memcpy (&(*init_http_ptr)->settings, &serv_addr, sizeof (serv_addr));

  if (bind (server_fd, (struct sockaddr *)&serv_addr, sizeof (serv_addr)) != 0)
    {
      printf ("Bind failed: %s \n", strerror (errno));
      return (1);
    }

  if (listen (server_fd, HTTP_SERVER_MAX_LISTEN_CLIENTS) != 0)
    {
      printf ("Listen failed: %s \n", strerror (errno));
      return (1);
    }

  (*init_http_ptr)->settings_len = sizeof (client_addr);

  return 0;
}

void
shutdown_http_server (Http_Info **init_http_ptr)
{
  close ((*init_http_ptr)->master_socket);
  free ((*init_http_ptr));
}
