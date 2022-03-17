#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <rocket_engine.h>
#include <rocket_executor.h>

#define DEFAULT_PORT 4224

// Maximum number of bytes in one message.
#define MAX_MSG_SIZE 4096
// Maximum number of concurrent connections.
#define MAX_NUM_CONN 4096

#define INT_TO_VOIDPTR(i) (void*)(uintptr_t)(i)

static int listen_on_port(unsigned short port) {
  // Create the listenng socket.
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
    return -1;
  }
  // Build the server's address.
  struct sockaddr_in serveraddr;
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons(port);
  // Allow restarting the server immediately after we kill it.
  int opt_val = 1;
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt_val,
                 sizeof(opt_val)) < 0) {
    fprintf(stderr, "Failed to configure listening socket: %s\n", strerror(errno));
    return -1;
  }
  // Associate the listening socket with the address.
  if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
    fprintf(stderr, "Failed to bind listening socket: %s\n", strerror(errno));
    return -1;
  }
  // Make the listening socket ready to accept connections.
  if (listen(listenfd, MAX_NUM_CONN) < 0) {
    fprintf(stderr, "Failed to listen on port %d: %s\n", port, strerror(errno));
    return -1;
  }

  return listenfd;
}

static void* echo(void* context) {
  int client_fd = (uintptr_t)context;
  char buf[MAX_MSG_SIZE];

  while (true) {
    ssize_t recv_bytes = recv(client_fd, buf, MAX_MSG_SIZE, /*flags=*/0);
    if (recv_bytes <= 0) {
      if (recv_bytes < 0) {
        fprintf(stderr, "Failed to recv message from client %d: %s\n",
                client_fd, strerror(errno));
      }
      break;
    }
    ssize_t send_bytes = send(client_fd, buf, recv_bytes, /*flags=*/0);
    if (send_bytes < 0) {
      fprintf(stderr, "Failed to send %ld bytes to client %d: %s\n",
              send_bytes, client_fd, strerror(errno));
      break;
    } else if (send_bytes != recv_bytes) {
      fprintf(stderr,
              "Failed to send all %ld bytes to client %d. Sent %ld bytes\n",
              recv_bytes, client_fd, send_bytes);
      break;
    }
  }
  return NULL;
}

static int run_echo_server(unsigned short port) {
  int listenfd = listen_on_port(port);
  if (listenfd < 0) {
    return -1;
  }

  while (true) {
    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    int clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddr_len);
    if (clientfd < 0) {
      fprintf(stderr, "Failed to accept connection\n");
      return -1;
    }
    pthread_t client_thread;
    int err = pthread_create(&client_thread, /*attr=*/NULL, echo,
                             INT_TO_VOIDPTR(clientfd));
    if (err != 0) {
      fprintf(stderr, "Failed to create a thread for client %d: %s\n", clientfd,
              strerror(err));
      return -1;
    }
    err = pthread_detach(client_thread);
    if (err != 0) {
      fprintf(stderr, "Failed to detach thread %lu: %s\n", client_thread,
              strerror(err));
      return -1;
    }
  }
  // Should never reach here if everything goes well.
  return 0;
}

static void* async_echo(void* context) {
  int client_fd = (uintptr_t)context;
  char buf[MAX_MSG_SIZE];

  while (true) {
    ssize_t recv_bytes = recv_await(client_fd, buf, MAX_MSG_SIZE, /*flags=*/0);
    if (recv_bytes <= 0) {
      if (recv_bytes < 0) {
        fprintf(stderr, "Failed to recv message from client %d\n",
                client_fd);
      }
      break;
    }
    ssize_t send_bytes = send_await(client_fd, buf, recv_bytes, /*flags=*/0);
    if (send_bytes < 0) {
      fprintf(stderr, "Failed to send %ld bytes to client %d\n", send_bytes,
              client_fd);
      break;
    } else if (send_bytes != recv_bytes) {
      fprintf(stderr,
              "Failed to send all %ld bytes to client %d. Sent %ld bytes\n",
              recv_bytes, client_fd, send_bytes);
      break;
    }
  }
  return NULL;
}

typedef struct {
  unsigned short port;
  rocket_executor_t *executor;
} async_echo_server_context_t;

static void* run_async_echo_server(void* context_in) {
  async_echo_server_context_t *context = context_in;
  unsigned short port = context->port;
  int listenfd = listen_on_port(port);
  if (listenfd < 0) {
    return INT_TO_VOIDPTR(-1);
  }

  while (true) {
    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    int clientfd = accept_await(listenfd, (struct sockaddr *)&clientaddr,
                                &clientaddr_len, /*flags=*/0);
    if (clientfd < 0) {
      fprintf(stderr, "Failed to accept connection");
      return INT_TO_VOIDPTR(-1);
    }
    rocket_executor_submit_task(context->executor, async_echo,
                                INT_TO_VOIDPTR(clientfd));
  }
  // Should never reach here if everything goes well.
  return INT_TO_VOIDPTR(0);
}

void usage(const char *program) {
  fprintf(stdout, "Usage: %s [-p <port>] [-a]\n", program);
  fprintf(stdout, "Options: \n");
  fprintf(stdout, "\t-p <port> The port to listen on\n");
  fprintf(stdout, "\t-a Enable asynchrnous I/O\n");
}

int main(int argc, char **argv) {
  int opt;
  int port = DEFAULT_PORT;
  bool async = false;
  while ((opt = getopt(argc, argv, "p:a")) != -1) {
    switch (opt) {
    case 'p':
      port = atoi(optarg);
      break;
    case 'a':
      async = true;
      break;
    default:
      usage(argv[0]);
      return -1;
      break;
    }
  }
  if (async) {
    fprintf(stdout, "Running async echo server ...\n");
    rocket_engine_t *engine =
        rocket_engine_create(/*queue_depth=*/MAX_NUM_CONN);
    rocket_executor_t *executor = rocket_executor_create(engine);

    async_echo_server_context_t context;
    context.port = port;
    context.executor = executor;

    rocket_executor_submit_task(executor, run_async_echo_server, &context);
    rocket_executor_execute(executor);

    // Should never reach here if everything goes well.
    rocket_executor_destroy(executor);
    rocket_engine_destroy(engine);
    return 0;
  } else {
    fprintf(stdout, "Running sync echo server ...\n");
    return run_echo_server(port);
  }
}
