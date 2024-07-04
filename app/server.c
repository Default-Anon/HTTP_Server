/*
 * ver 1.0.0
 * HTTP_SERVER FULL With Gzip for echo query
 *
 */

#include "headers/utils.h"
#include <signal.h>

static int signal_status = 1;
void
sig_int_handler()
{
  signal_status = 0;
}

void
usage()
{
  puts("You must run program with directory flag");
  puts("Example:");
  puts("\t<http-server-exe> --direcory {PATH}");
}

int
main(int argc, char** argv)
{
  char* path = "/tmp/";
  Http_Info* info;
  int sock_counter = 0, max_desk;
  fd_set master;
  struct timeval time;
  time.tv_usec = 100;
  time.tv_sec = 0;
  // Disable output buffering
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);
  // handle ctrl-c interrupt
  signal(SIGINT, sig_int_handler);

  if (argc < 3) {
    usage();
  } else {
    path = argv[2];
  }

  if (init_http_server(&info))
    return -1;
  FD_ZERO(&master);
  FD_SET(info->master_socket, &master);
  max_desk = info->master_socket;

  while (signal_status) {
    fd_set slaves_desk;
    FD_ZERO(&slaves_desk);
    slaves_desk = master;
    if (select(max_desk + 1, &slaves_desk, 0, 0, &time) == -1) {
      printf("select():\t%d,%s\n", errno, strerror(errno));
      shutdown_http_server(&info);
      return 0;
    }
    for (int i = 0; i <= max_desk; i++) {
      if (FD_ISSET(i, &slaves_desk)) {
        if (i == info->master_socket) {
          if (handle_new_client_connection(info, &master, &max_desk, path))
            return -1;
        } else {
          if (handle_request(info, &master, i, path))
            return -1;
        }
      }
    }
  }
  shutdown_http_server(&info);
  return 0;
}
