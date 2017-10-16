/* Methods called in http.l to further parse HTTP */
#include "proxy.h"

//Functions
/* -------------------------------------------------------------------------------------------------------*/

void parse_method(){
  printf("Method \n");
}
/* -------------------------------------------------------------------------------------------------------*/

void parse_header(struct request_t *headers, char *header_line) {
  // printf("%s \n %s \n", headers[0].header_name, headers[0].header_value);
  // printf("Header: ");
  char *saveptr = header_line;
  char log_message[LOG_SIZE];

  char* name;

  //TODO: This process will have to be done in a loop that iterates through the array of headers (look through entire array, check if values are null). Implement this after getting the parsing right.

  /* Parse the header */
  // printf("%s", header_line);

  name = strtok_r(saveptr, ":", &saveptr);
  if(name == NULL) {
    strncpy(log_message, "Failure: Parsing HTTP", LOG_SIZE);
    log_event(log_message);
    // respond('400 BAD REQUEST');
    pthread_exit(NULL);
  }

  printf("%s%s", name, saveptr);
  /* HTTP header name saved in name
    HTTP header value saved in saveptr*/

  /* malloc() enough memory for the name and value */

  /* Assign name and value into the structure. */
  // headers[0].header_name = (char *) malloc(strlen(name));

}
