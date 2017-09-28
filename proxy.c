/* Functionality of Proxy web server. */
#include <stdio.h>
#include <stdlib.h>
#include "proxy.h"

//Function Prototypes
static int read_request(void);
static int parse_request(void);
static int authenticate(void);
static int connect_to_host(void);
static int send_request_to_host(void);
static int get_response(void);
static int respond(void);

//Functions
/* -------------------------------------------------------------------------------------------------------*/

int read_request(void){

  printf("Read Request: ");

  int success = rand() % 2;

  if (success == 0){
    int prototype = rand() % 2;

    if (prototype == 0){
      printf("Prototype \n");
      respond(); }

    else{
      printf("Success \n");
      parse_request();
     }
  }

  else{
    printf("Failure \n");
    log_result();
   }

  return EXIT_SUCCESS;
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
        respond();
      }

      else{
        printf("Success \n");
        connect_to_host();
      }
    }

    else {
      printf("Relative Scope \n");
      authenticate();
     } //Relative

  }

  else{
    printf("Failure \n");
    respond();
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
    respond();
   }

  return EXIT_SUCCESS;
}
/* -------------------------------------------------------------------------------------------------------*/

static int authenticate(void){

  printf("Authenticate: ");

  int request_type = rand() % 2;
  if (request_type == 0){
    printf("Stop \n");
    return EXIT_SUCCESS;
   }

  else{
    printf("Other \n");
    respond();
   }

  return EXIT_SUCCESS;

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
    respond();
   }

  return EXIT_SUCCESS;
}
/* -------------------------------------------------------------------------------------------------------*/

static int get_response(void){

  printf("Get Response: ");

  int success = rand() % 2;
  if (success == 0){
    printf("Success \n");
    respond();
   }

  else{
    printf("Failure \n");
    respond();
   }

  return EXIT_SUCCESS;
}
/* -------------------------------------------------------------------------------------------------------*/

static int respond(void){

  printf("Respond: ");

  int success = rand() % 2;
  if (success == 0){
    printf("Success \n");
    log_result(); }

  else{
    printf("Failure \n");
    log_result();
   }

  return EXIT_SUCCESS;
}
/* -------------------------------------------------------------------------------------------------------*/

void *serve_request(void *thread_info) {

  int client = (int) thread_info;

  printf("New thread! \n Client: %d \n", client);

  //Called every time a thread is created.
  //Equivalent of threadRoutine in my-server.c
  pthread_exit(NULL);
}
