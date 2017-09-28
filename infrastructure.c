/* Infrastucture of proxy web server. */
#include <stdio.h>
#include <stdlib.h>
#include "proxy.h"
#include <time.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


//Function Prototypes
static int start_server(int port);
static int accept_connection(int server);
static int wait_for_request(int server);

//Global Variable(s)
int master_pid;

//Functions
/* -------------------------------------------------------------------------------------------------------*/

int main(void){

  srand(time(NULL));

  int port = 8080;
  int server;

  /* Start a new server */
  if (-1 == (server = start_server(port))) {
    perror("Starting server");
    return EXIT_FAILURE;
  }
  // TODO: Log server started.

  accept_connection(server);

  // /* Finally,stop the server */
  // if (-1 == stop_server(server)) {
  //   perror("Stopping server");
  //   return EXIT_FAILURE;
  // }

  return EXIT_SUCCESS;
}
/* -------------------------------------------------------------------------------------------------------*/

static int start_server(int port){

  int server;
  struct sockaddr_in serv_addr;

  /* Create a new TCP/IP socket */
  if (-1 == (server = socket(AF_INET, SOCK_STREAM, 0)))
    return -1;

  /* Prepare the endpoint */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);

  /* Bind the endpoint to the socket */
  if (-1 == bind(server, (struct sockaddr *) &serv_addr, sizeof(serv_addr)))
    return -1;

  /* Prepare for listening */
  if (-1 == listen(server, 5))
    return -1;

  return server;
}
/* -------------------------------------------------------------------------------------------------------*/

static int accept_connection(int server){

  int client;
  int done = 0; // NOTE: This value will only change if the errno from accept returning -1 is set to EINTR (happens w/ SIGUSR1).
  pthread_t newThread = NULL;

  do {

    /* Wait for the next request */
    if (-1 == (client = wait_for_request(server))) {
      perror("Accepting requests");
      // TODO: Check here if errno is set to EINTR, meaning a SIGUSR1 has been received and server must be stopped.
      // If EINTR is set, log server has been stopped. If not, server has been stopped due to error.
      printf("Exitting the server.");
      return EXIT_FAILURE;
    }

    long clientArg = (long) client;
    if ( 0 != pthread_create(&newThread, NULL, serve_request, (void *) clientArg) )
    {
      perror("Error creating thread.");
      // TODO: log_result("Error creating thread");
      break;
    }

  } while (!done);

  return EXIT_SUCCESS;
}
/* -------------------------------------------------------------------------------------------------------*/
static int wait_for_request(int server) {
  struct sockaddr_in cli_addr;
  socklen_t clilen = sizeof(cli_addr);

  /* Accept a request and get a new connection for it */
  return accept(server, (struct sockaddr *) &cli_addr, &clilen);
}
/* -------------------------------------------------------------------------------------------------------*/

int log_result(void){

  printf("Log Result \n");
  return EXIT_SUCCESS;

}
