/* Prototype of proxy web server. */

#include "proxy.h"
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

//Function Prototypes
static int start_server(int port);
static int accept_connection(int server);
static int wait_for_request(int server);
static void signal_handler(int signum);

//Global Variable(s)
int master_pid;

//Functions
/* -------------------------------------------------------------------------------------------------------*/
/* Starts server, and calls accept_connection() */
int main(void){

  srand(time(NULL));

  int port = 8080;
  int server;

  /* Start a new server */
  if (EXIT_FAILURE == (server = start_server(port))) {
    return EXIT_FAILURE;
  }

  log_event("Proxy Server Started.");

  accept_connection(server);

  log_event("Proxy Server Terminated.");

  return EXIT_SUCCESS;
}
/* -------------------------------------------------------------------------------------------------------*/
/* Starts the server and prepares it for listening. */
static int start_server(int port){

  int server;
  struct sockaddr_in serv_addr;

  /* Create a new TCP/IP socket */
  if (-1 == (server = socket(AF_INET, SOCK_STREAM, 0)) ) {
    log_event("Failure: Create Socket.");
    return EXIT_FAILURE; }

  /* Prepare the endpoint */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);

  /* Bind the endpoint to the socket */
  if (-1 == bind(server, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) {
    log_event("Failure: Busy Socket (wait and retry).");
    return EXIT_FAILURE; }

  /* Prepare for listening */
  if (-1 == listen(server, 5)) {
    log_event("Failure: Prepare for Listening.");
    return EXIT_FAILURE; }

  return server;
}
/* -------------------------------------------------------------------------------------------------------*/
/* Main loop of program. Accepts connections and creates threads. */
static int accept_connection(int server){

  int client;
  int done = 0;
  pthread_t newThread = NULL;
  FILE *pidFile;
  struct sigaction sa;

  /* Set global variable */
  master_pid = getpid();

  /* Open proxy.pid */
  pidFile = fopen("/tmp/proxy.pid", "w");
  if (pidFile == NULL) {
    log_event("Failure: Open proxy.pid");
    return EXIT_FAILURE;
    }

  /* Write master thread pid into proxy.pid */
  if ( fprintf(pidFile, "%d", master_pid ) < 0){
    log_event("Failure: Write to proxy.pid");
    return EXIT_FAILURE;
    }

  fclose(pidFile);

  /* Configure signal handler */
  sa.sa_handler = signal_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGUSR1, &sa, NULL) == -1) {
    log_event("Failure: SIGUSR1");
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
        log_event("Failure: Accepting Requests");
        stop_server(server);
        return EXIT_FAILURE;
      }
    }

    long clientArg = (long) client;
    if ( 0 != pthread_create(&newThread, NULL, serve_request, (void *) clientArg) )
    {
      log_event("Failure: Thread Creation");
    }

  } while (!done);

  return EXIT_FAILURE;
}
/* -------------------------------------------------------------------------------------------------------*/
/* Accept a request and get a new connection for it */
static int wait_for_request(int server) {
  struct sockaddr_in cli_addr;
  socklen_t clilen = sizeof(cli_addr);

  return accept(server, (struct sockaddr *) &cli_addr, &clilen);
}
/* -------------------------------------------------------------------------------------------------------*/
/* We only want SIGUSR1 to trigger accept() to return */
static void signal_handler( __attribute__((unused)) int signum) {
    return;
}
/* -------------------------------------------------------------------------------------------------------*/
/* TODO: Properly log events, rather than printing to stdout. */
void log_event(char *log_message){

  printf("%s \n", log_message);
  return;

}
