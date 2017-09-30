/* Functionality of Proxy web server. */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "proxy.h"

//Function Prototypes
static void read_request(int client, char *request_buffer);
static int parse_request(void);
static void authenticate(int client, char *request_buffer);
static int connect_to_host(void);
static int send_request_to_host(void);
static int get_response(void);
static void respond(int client, char *content);

//Functions
/* -------------------------------------------------------------------------------------------------------*/

static void read_request(int client, char *request_buffer){

  char log_message[LOG_SIZE];
  int n;

  /* Read the entire HTTP request */
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

static void authenticate(int client, char *request_buffer){

  //TODO: Check if its a stop request, currently assumes any admin request is a stop.

  respond(client, request_buffer);
  //TODO: Send signal to stop server.
  //NOTE: Final version won't respond before stopping.

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

static void respond(int client, char *content){

  char log_message[LOG_SIZE];
	if (-1 == write(client, content, strlen(content))) {
    strncpy(log_message, "Failure: Respond to Client", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
    }
  return;
}
/* -------------------------------------------------------------------------------------------------------*/

void *serve_request(void *thread_info) {

  int client = (int) thread_info;
  char request_buffer[REQUEST_SIZE];
  char request_buffer_final[REQUEST_SIZE + 100];

  printf("New thread! Client: %d \n", client);
  //Called every time a thread is created.
  //Equivalent of threadRoutine in my-server.c

  strcpy(request_buffer_final,"HTTP/1.x 200 OK\nContent-Type: text/html\n\n" );

  int request = rand() % 2;
  if (request == 0){
    printf("Display Request \n");

    // NOTE: The following is a quick and dirty way to write sample content, simply to display something on the prototype. The finished version should more efficiently concatenate the actual content with the response/content type.

    read_request(client, request_buffer);
    strcat(request_buffer_final, request_buffer);
    respond(client, request_buffer_final);

    pthread_exit(NULL);
  }

  else{
    printf("Stop Server \n");
    strcat(request_buffer_final, "Server Stopped.");
    authenticate(client, request_buffer_final);
    pthread_exit(NULL);
   }

  pthread_exit(NULL);
}
