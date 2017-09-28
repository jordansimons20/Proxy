/* Infrastucture of proxy web server. */
#include <stdio.h>
#include <stdlib.h>
#include "proxy.h"
#include <time.h>

//Function Prototypes
static int start_server(void);
static int accept_connection(void);

//Functions
/* -------------------------------------------------------------------------------------------------------*/

int main(void){
  srand(time(NULL));
  start_server();

  return EXIT_SUCCESS;
}
/* -------------------------------------------------------------------------------------------------------*/

static int start_server(void){
  printf("Start Server \n");
  accept_connection();

  return EXIT_SUCCESS;
}
/* -------------------------------------------------------------------------------------------------------*/

static int accept_connection(void){
  printf("Accept Connection: ");

  int success = rand() % 2;
  if (success == 0){
    printf("Success \n");
    read_request();
    //Create thread, call serve_request.
   }

  else{
    printf("Failure \n");
    log_result();
   }

  return EXIT_SUCCESS;
}
/* -------------------------------------------------------------------------------------------------------*/

int log_result(void){
  printf("Log Result \n");
  return EXIT_SUCCESS;

  // In the actual program, this will call accept_connection() again after logging.
}
