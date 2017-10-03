/* Functionality of Proxy web server. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "proxy.h"

//Function Prototypes
static void read_request(int client, char *request_buffer);
static int parse_request(void);
static void authenticate();
static int connect_to_host(void);
static int send_request_to_host(void);
static int get_response(void);
static void respond(int client, char *content);

//Functions
/* -------------------------------------------------------------------------------------------------------*/
/* Read the entire HTTP request */
static void read_request(int client, char *request_buffer){

  char log_message[LOG_SIZE];
  int n;

  if (-1 == (n = read(client, request_buffer, REQUEST_SIZE - 1))) {
    strncpy(log_message, "Failure: Read HTTP Request", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }

  request_buffer[n] = 0;

  return;
}
/* -------------------------------------------------------------------------------------------------------*/

static int parse_request(void){

  printf("Parse Request: ");
  int success = rand() % 2;

  if(success == 0){
    int request_scope = rand() % 2;

    if(request_scope == 0) { //Asbolute
      int blacklisted = rand() % 2;

      if (blacklisted == 0){ //Blacklisted
        printf("Blacklisted \n");
        // respond();
      }

      else{
        printf("Success \n");
        connect_to_host();
      }
    }

    else {
      printf("Relative Scope \n");
      // authenticate();
     } //Relative

  }

  else{
    printf("Failure \n");
    // respond();
  }

  return EXIT_SUCCESS;
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
static void authenticate(){

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
static void respond(int client, char *content){

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
  char request_buffer_final[REQUEST_SIZE + 100];

  strcpy(request_buffer_final,"HTTP/1.x 200 OK\nContent-Type: text/html\n\n" );

  int request = rand() % 2;
  if (request == 0){

    // NOTE: The following is a quick and dirty way to write sample content, simply to display something on the prototype. The finished version should more efficiently concatenate the actual content with the response/content type.

    read_request(client, request_buffer);
    strcat(request_buffer_final, request_buffer);
    respond(client, request_buffer_final);

    pthread_exit(NULL);
  }

  else{
    authenticate();
    pthread_exit(NULL);
   }

  pthread_exit(NULL);
}
