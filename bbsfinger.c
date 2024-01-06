#include "defs.h"
#include "ext.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define FINGER_PORT 79
#define MAX_BUFFER_SIZE 1024

static void s_sigalrm2(int signo) {
  if (signo == SIGALRM) {
    signal(SIGALRM, s_sigalrm2);
    alarm(30);
  }
}

void handle_client(int client_socket, struct sockaddr_in client_address) {
  char request[MAX_BUFFER_SIZE];


  // Receive request from client
  memset(request, 0, MAX_BUFFER_SIZE / 2);
  if (recv(client_socket, request, sizeof(request), 0) == -1) {
    perror("Error receiving request from client");
    errlog("Error receiving request from client");
    close(client_socket);
    return;
  }

  dup2(client_socket, 1);

  if (check_ip_blocklist(client_address.sin_addr.s_addr)) {
    my_printf("\n*** Your IP address has been permanently blocked.\n\n");
    errlog("Blcoked connection from %s", inet_ntoa(client_address.sin_addr));
    close(client_socket);
    return;
  }

  if (request[0] == '\n' || request[0] == '\r') {
    show_online(2);
    errlog("Sent wholist to %s", inet_ntoa(client_address.sin_addr));
  } else {
    request[strlen(request) - 2] = 0;
    errlog("Profile request '%s' from %s", request, inet_ntoa(client_address.sin_addr));
    if (profile(request, NULL, PROF_REG) < 0) {
      printf("There is no user %s on this BBS.\n", request);
    }

  }

  close(1);
  shutdown(client_socket, 1);
}

void bbsfinger(void) {
  int listen_socket, client_socket;
  struct sockaddr_in server_address, client_address;
  socklen_t client_address_len;

  // Create socket
  if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Error creating socket");
    exit(1);
  }

  if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1},
                 sizeof(int)) < 0) {
    perror("setsockopt(SO_REUSEADDR) failed");
  }

  ouruser = getuser("Guest");

  if (!ouruser) {
    printf("!ouruser\n");
    _exit(1);
  }

  // Set up server address
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(FINGER_PORT);

  // Bind socket to the server address
  if (bind(listen_socket, (struct sockaddr *)&server_address,
           sizeof(server_address)) == -1) {
    perror("Error binding socket");
    exit(1);
  }

  // Listen for client connections
  if (listen(listen_socket, 5) == -1) {
    perror("Error listening for connections");
    exit(1);
  }

  printf("Finger daemon is running. Waiting for connections...\n");

  close(0);
  close(1);

  setsid();

  signal(SIGPIPE, SIG_IGN);
  signal(SIGALRM, s_sigalrm2);
  alarm(30);

  rows = 9999;
  ansi = 0;

  // Accept and handle client connections
  while (1) {
    client_address_len = sizeof(client_address);

    // Accept client connection
    if ((client_socket =
             accept(listen_socket, (struct sockaddr *)&client_address,
                    &client_address_len)) == -1) {
      perror("Error accepting connection");
      continue;
    }

    errlog("Accepted connection from %s", inet_ntoa(client_address.sin_addr));

    // Handle client request
    handle_client(client_socket, client_address);
  }

  close(listen_socket);

  exit(0);
}
