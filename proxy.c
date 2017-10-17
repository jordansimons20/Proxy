/* Functionality of Proxy web server. */
#include "proxy.h"
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>

//Function Prototypes
static void read_request(int client, char *request_buffer);
static int connect_to_host(void);
static int send_request_to_host(void);
static int get_response(void);

//Functions
/* -------------------------------------------------------------------------------------------------------*/
/* Read the entire HTTP request */
static void read_request(int client, char *request_buffer){

  char log_message[LOG_SIZE];
  int n;
  // int request_length = REQUEST_SIZE;
  // int read_length = 0;
  // int opts;

  if (-1 == (n = read(client, request_buffer, REQUEST_SIZE - 1))) {
    strncpy(log_message, "Failure: Read HTTP Request", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }

  request_buffer[n] = 0;

  return;

  /* NOTE: Dynamic reading incomplete:
        -read/recv hangs or errors out
        - Need to implement actual realloc() if the loop works.
        - Maybe try select()?*/

  // opts = fcntl(client, F_SETFL, O_NONBLOCK);
  // // opts &= ~O_NONBLOCK;
  //
  // /* Loop until we read the entire request */
  // // while (0 != (n = read(client, request_buffer, REQUEST_SIZE - 1))) {
  // while (-1 != (n = recv(client, request_buffer, 100- 1, 0))) {
  //
  //   /* Keep track of position for terminating character */
  //   read_length += n;
  //   printf("Bytes read: %d \n", n);
  // }
  //
  //
  //
  // if(n == -1 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
  //   request_buffer[read_length] = 0;
  //   printf("inside if \n");
  // }
  //
  // else{
  //   strncpy(log_message, "Failure: Read HTTP Request", LOG_SIZE);
  //   log_event(log_message);
  //   free(request_buffer);
  //   pthread_exit(NULL);
  // }
  //
  // printf("help \n");
  //
  // return;
}
/* -------------------------------------------------------------------------------------------------------*/
static int connect_to_host(void){

  printf("Connect to Host: ");

  int success = rand() % 2;
  if (success == 0){
    printf("Success \n");
    send_request_to_host();
   }

  else{
    printf("Failure \n");
    // respond();
   }

  return EXIT_SUCCESS;
}
/* -------------------------------------------------------------------------------------------------------*/
/* Authenticate access to admin functions, and fulfill them. */
void authenticate(){

  //TODO: Check if its a stop request, currently assumes any admin request is a stop.

  /* Send signal to stop server, using global variable master_pid */
  kill(master_pid, SIGUSR1);

  return;
}
/* -------------------------------------------------------------------------------------------------------*/

static int send_request_to_host(void){

  printf("Send Request to Host: ");

  int success = rand() % 2;
  if (success == 0){
    printf("Success \n");
    get_response();
   }

  else{
    printf("Failure \n");
    // respond();
   }

  return EXIT_SUCCESS;
}
/* -------------------------------------------------------------------------------------------------------*/

static int get_response(void){

  printf("Get Response: ");

  int success = rand() % 2;
  if (success == 0){
    printf("Success \n");
    // respond();
   }

  else{
    printf("Failure \n");
    // respond();
   }

  return EXIT_SUCCESS;
}
/* -------------------------------------------------------------------------------------------------------*/
/* Responds to client with given content. */
void respond(int client, char *content){

  // NOTE: Might work better to just pass *content with the HTML header specified, then strcat whatever else.

  char log_message[LOG_SIZE];
	if (-1 == write(client, content, strlen(content))) {
    strncpy(log_message, "Failure: Respond to Client", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
    }
  return;
}
/* -------------------------------------------------------------------------------------------------------*/
/* Called every time a thread is created. */
void *serve_request(void *thread_info) {

  int client = (int) thread_info;
  char request_buffer[REQUEST_SIZE];
  struct request_t http_request;

  read_request(client, request_buffer);
  parse_request(client, request_buffer, &http_request);

  //TODO: Free all malloc()'d memory.

  pthread_exit(NULL);
}
