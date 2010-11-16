#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "core/socket_loop.h"
#include "core/log.h"

static void client_on_connect(SocketLoop *loop, int fd);
static void client_on_message(SocketLoop *loop, int fd, const char *message);
static void client_on_disconnect(SocketLoop *loop, int fd);

static void server_on_connect(SocketLoop *loop, int fd);
static void server_on_message(SocketLoop *loop, int fd, const char *message);
static void server_on_disconnect(SocketLoop *loop, int fd);

const SocketLoopEventHandler client = 
{
  client_on_connect,
  client_on_message,
  client_on_disconnect
};

const SocketLoopEventHandler server = 
{
  server_on_connect,
  server_on_message,
  server_on_disconnect
};

#define MESSAGESIZE 65536
#define NMESSAGES 50
char testmsg[MESSAGESIZE+1];

int main()
{
  int fds[2];
  int fd;
  size_t i;
  pid_t pid;
  const SocketLoopEventHandler *handler;
  SocketLoop *loop;

  /* Set up a connection */
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) 
    fatal("socketpair error: %s", strerror(errno));

  message("Preparing test message of size %d", MESSAGESIZE);
  for (i=0; i<MESSAGESIZE; i++)
    testmsg[i] = 'a' + rand()%26;
  testmsg[MESSAGESIZE] = '\0';

  /* Set up processes */
  if ((pid = fork()) != 0)
  {
    message("Client: %d", getpid());
    message("Server: %d", pid);
    close(fds[1]);
    fd = fds[0];
    handler = &client;
  }
  else
  {
    close(fds[0]);
    fd = fds[1];
    handler = &server;
  }

  /* Main event loop */
  loop = socketloop_new(handler);
  socketloop_add_client(loop, fd);
  socketloop_run(loop);
  socketloop_delete(loop);

  if (pid != 0)
  {
    int status;
    message("Waiting for server to shutdown...");
    if (wait(&status) >= 0)
      message("Ok, server down.");
    else
      warning("wait() failed");
  }
  return 0;
}


/* 
 * Client callbacks
 */
static void client_on_connect(SocketLoop *loop, int fd)
{
  int i;

  message("Client: connect %d", fd);
  message("Sending %d messages of length %d", NMESSAGES, MESSAGESIZE);
  for (i=0; i<NMESSAGES; i++)
    socketloop_send(loop, fd, testmsg);
}

static void client_on_message(SocketLoop *loop, int fd, const char *msg)
{
  int n;
  int len;
  char cmd[10];
  static int n_ok = 0;

  trace("Server reply: %s", msg);
  sscanf(msg, "%d %s length=%d", &n, cmd, &len);
  if (strcmp(cmd, "OK") == 0)
  {
    if (len==MESSAGESIZE)
      n_ok++;
    else
      warning("Message %d reported as OK, but lengths mismatch", n, len);
  }
  else if (strcmp(cmd, "FAIL") == 0)
    warning("Message %d failed, length %d", n, len);
  else if (strcmp(cmd, "DONE") == 0)
    message("Sequence complete, %d/%d passed", n_ok, NMESSAGES);
}

static void client_on_disconnect(SocketLoop *loop, int fd)
{
  message("Client: disconnect %d", fd);
  message("Client shutting down");
  socketloop_stop(loop);
}


/*
 * Server callbacks 
 */

static void server_on_connect(SocketLoop *loop, int fd)
{
}

static void server_on_message(SocketLoop *loop, int fd, const char *msg)
{
  char buf[100];
  int ok = (strcmp(msg, testmsg) == 0);
  static unsigned int counter = 1;

  sprintf(buf, "%u %s length=%zu", counter++, ok?"OK":"FAIL", strlen(msg));
  socketloop_send(loop, fd, buf);
  if (counter > NMESSAGES)
  {
    sprintf(buf, "%d DONE Server shutting down...", counter);
    socketloop_send(loop, fd, buf);
    socketloop_drop_client(loop, fd);
  }
}

static void server_on_disconnect(SocketLoop *loop, int fd)
{
  socketloop_stop(loop);
}
