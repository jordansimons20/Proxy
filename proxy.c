/* proxy.c, by Jordan Simons. Skeleton of a program to create a proxy web server. */
#include <stdio.h>
#include <stdlib.h>
#include "proxy.h"
#include <time.h>

///////////////////////////////////////////////////////////////////////////////////////////////

int main(void){
  srand(time(NULL));
  start_server();

  return EXIT_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////

int start_server(void){
  printf("Start Server \n");
  accept_connection();

  return EXIT_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////

int accept_connection(void){
  printf("Accept Connection: ");

  int success = rand() % 2;
  if (success == 0){
    printf("Success \n");
    read_request();
   }

  else{
    printf("Failure \n");
    log_result();
   }

  return EXIT_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////

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
///////////////////////////////////////////////////////////////////////////////////////////////

int parse_request(void){
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
///////////////////////////////////////////////////////////////////////////////////////////////
int connect_to_host(void){
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
///////////////////////////////////////////////////////////////////////////////////////////////
int authenticate(void){
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
///////////////////////////////////////////////////////////////////////////////////////////////
int send_request_to_host(void){
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
///////////////////////////////////////////////////////////////////////////////////////////////
int get_response(void){
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
///////////////////////////////////////////////////////////////////////////////////////////////

int log_result(void){
  printf("Log Result \n");
  return EXIT_SUCCESS;

  // In the actual program, this will call accept_connection() again after logging.
}
///////////////////////////////////////////////////////////////////////////////////////////////

int respond(void){
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
