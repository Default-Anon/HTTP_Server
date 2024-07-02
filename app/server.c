#include "headers/utils.h"
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
static int signal_status = 1;
void
sig_int_handler ()
{
  signal_status = 0;
}

int
main ()
{

  Http_Info *info;
  int sock_counter = 0, max_desk;
  fd_set master;
  // Disable output buffering
  setbuf (stdout, NULL);
  setbuf (stderr, NULL);
  // handle ctrl-c interrupt
  signal (SIGINT, sig_int_handler);
  const char *RESPONSE_IF_CONNECTED = "HTTP/1.1 200 OK\r\n\r\n";
  /* You can use print statements as follows for debugging, they'll be
   visible when running tests. */
  printf ("Logs from your program will appear here!\n");

  if (init_http_server (&info))
    return -1;
  FD_ZERO (&master);
  FD_SET (info->master_socket, &master);
  max_desk = info->master_socket;
  while (signal_status)
    {
      fd_set slaves_desk;
      FD_ZERO (&slaves_desk);
      slaves_desk = master;
      if (select (max_desk + 1, &slaves_desk, 0, 0, 0) == -1)
        {
          printf ("select():\t%d,%s", errno, strerror (errno));
          return -1;
        }
      for (int i = 0; i <= max_desk; i++)
        {
          if (FD_ISSET (i, &slaves_desk))
            {
              if (i == info->master_socket)
                {
                  int sock = accept (info->master_socket,
                                     (struct sockaddr *)&info->settings,
                                     (socklen_t *)&info->settings_len);
                  if (sock == -1)
                    {
                      printf ("accept():\t%d,%s\n", errno, strerror (errno));
                      break;
                    }
                  FD_SET (sock, &master);
                  max_desk = sock > max_desk ? sock : max_desk;
                  int sent_bytes = send (sock, RESPONSE_IF_CONNECTED,
                                         strlen (RESPONSE_IF_CONNECTED), 0);
                }
              else
                {
                  // do something
                }
            }
        }
    }
  shutdown_http_server (&info);

  return 0;
}
