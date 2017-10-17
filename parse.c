/* Methods called in http.l to further parse HTTP */
#include "proxy.h"

//Functions
/* -------------------------------------------------------------------------------------------------------*/

void parse_method(){
  printf("Method \n");
}
/* -------------------------------------------------------------------------------------------------------*/

void parse_header(struct request_t *headers, char *header_line) {
  char *saveptr = header_line;
  char log_message[LOG_SIZE];
  char* name;

  //TODO: Trim extra white space (in header_value, i think).

  /* Parse the header. */
  name = strtok_r(saveptr, ":", &saveptr);
  if(name == NULL) {
    strncpy(log_message, "Failure: Parsing HTTP", LOG_SIZE);
    log_event(log_message);
    // respond('400 BAD REQUEST');
    pthread_exit(NULL);
  }

  /* Look for empty space in array of headers. */
  for(int i = 0; i < HEADER_ARRAY_LENGTH; i++) {

    if(headers[i].header_name == NULL && headers[i].header_value == NULL) {

      /* Assign name and value into the structure. */
      headers[i].header_name = (char *) malloc(strlen(name));
      if (headers[i].header_name == NULL) {
        strncpy(log_message, "Failure: malloc() header_name", LOG_SIZE);
        log_event(log_message);
        pthread_exit(NULL);
      }
      strcpy(headers[i].header_name, name);

      headers[i].header_value = (char *) malloc(strlen(saveptr));
      if (headers[i].header_value == NULL) {
        strncpy(log_message, "Failure: malloc() header_value", LOG_SIZE);
        log_event(log_message);
        pthread_exit(NULL);
      }
      strcpy(headers[i].header_value, saveptr);

      break;
    }
  }
}
