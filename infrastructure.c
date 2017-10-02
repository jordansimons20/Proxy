/* Prototype of proxy web server. */

#include <stdio.h>
#include <stdlib.h>
#include "proxy.h"
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>


//Function Prototypes
static int start_server(int port);
static int accept_connection(int server);
static int wait_for_request(int server);
static int stop_server(int server);
static void signal_handler(int signum);

//Global Variable(s)
int master_pid;

//Functions
/* -------------------------------------------------------------------------------------------------------*/

int main(void){

  srand(time(NULL));

  int port = 8080;
  int server;
  char log_message[LOG_SIZE];

  /* Start a new server */
  if (-1 == (server = start_server(port))) {
    strncpy(log_message, "Failure: Start the Proxy Server.", LOG_SIZE);
    log_event(log_message);
    return EXIT_FAILURE;
  }

  strncpy(log_message, "Proxy Server Started.", LOG_SIZE);
  log_event(log_message);

  accept_connection(server);

  strncpy(log_message, "Proxy Server Terminated.", LOG_SIZE);
  log_event(log_message);

  return EXIT_SUCCESS;
}
/* -------------------------------------------------------------------------------------------------------*/

static int start_server(int port){

  int server;
  struct sockaddr_in serv_addr;
  char log_message[LOG_SIZE];

  /* Create a new TCP/IP socket */
  if (-1 == (server = socket(AF_INET, SOCK_STREAM, 0)) ) {
    strncpy(log_message, "Failure: Create Socket.", LOG_SIZE);
    log_event(log_message);
    return EXIT_FAILURE; }

  /* Prepare the endpoint */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);

  /* Bind the endpoint to the socket */
  if (-1 == bind(server, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) {
    strncpy(log_message, "Failure: Bind the Endpoint to the Socket", LOG_SIZE);
    log_event(log_message);
    return EXIT_FAILURE; }

  /* Prepare for listening */
  if (-1 == listen(server, 5)) {
    strncpy(log_message, "Failure: Prepare for Listening.", LOG_SIZE);
    log_event(log_message);
    return EXIT_FAILURE; }

  return server;
}
/* -------------------------------------------------------------------------------------------------------*/

static int accept_connection(int server){

  int client;
  int done = 0;
  pthread_t newThread = NULL;
  FILE *pidFile;
  char log_message[LOG_SIZE];
  struct sigaction sa;

  /* Set global variable */
  master_pid = getpid();

  /* Open proxy.pid */
  pidFile = fopen("/tmp/proxy.pid", "w");
  if (pidFile == NULL) {
    strncpy(log_message, "Failure: Open proxy.pid", LOG_SIZE);
    log_event(log_message);
    return EXIT_FAILURE;
    }

  /* Write master thread pid into proxy.pid */
  if ( fprintf(pidFile, "%d", master_pid ) < 0){
    strncpy(log_message, "Failure: Write to proxy.pid", LOG_SIZE);
    log_event(log_message);
    return EXIT_FAILURE;
    }

  fclose(pidFile);

  /* Configure signal handler */
  sa.sa_handler = signal_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGUSR1, &sa, NULL) == -1) {
    strncpy(log_message, "Failure: SIGUSR1", LOG_SIZE);
    log_event(log_message);
    return EXIT_FAILURE;
  }


  do {

    /* Wait for the next request */
    if (-1 == (client = wait_for_request(server))) {

      /* Check for SIGUSR1 by checking if errno is set to EINTR */
      if(errno == EINTR) {
        stop_server(server);
        return EXIT_SUCCESS;
      }

      /* If we did not return from SIGUSR1, we have some unexpected error. */
      else {
        strncpy(log_message, "Failure: Accepting Requests", LOG_SIZE);
        log_event(log_message);
        stop_server(server);
        return EXIT_FAILURE;
      }
    }

    long clientArg = (long) client;
    if ( 0 != pthread_create(&newThread, NULL, serve_request, (void *) clientArg) )
    {
      strncpy(log_message, "Failure: Thread Creation", LOG_SIZE);
      log_event(log_message);
    }

  } while (!done);

  return EXIT_FAILURE;
}
/* -------------------------------------------------------------------------------------------------------*/
static int wait_for_request(int server) {
  struct sockaddr_in cli_addr;
  socklen_t clilen = sizeof(cli_addr);

  /* Accept a request and get a new connection for it */
  return accept(server, (struct sockaddr *) &cli_addr, &clilen);
}
/* -------------------------------------------------------------------------------------------------------*/

static int stop_server(int server) {
  /* Close the socket */
  return close(server);
}
/* -------------------------------------------------------------------------------------------------------*/

static void signal_handler(int signum) {
    /* We only want SIGUSR1 to trigger accept() to return */
    return;
}
/* -------------------------------------------------------------------------------------------------------*/

void log_event(char *log_message){

  printf("%s \n", log_message);
  // TODO: Properly log events, rather than printing to stdout.
  return;

}
