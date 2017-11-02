/* Methods called in http.l to further parse HTTP */
#include "proxy.h"
#include <errno.h>
//Functions
/* -------------------------------------------------------------------------------------------------------*/
/* Parse HTTP response status line */
void parse_status_line(struct status_line *status_line, char *http_line) {
  char log_message[LOG_SIZE];
  char *saveptr = http_line;
  char *protocol;
  char *code;
  char *reason;

  /* Parse the HTTP-Version. */
  protocol = strtok_r(saveptr, " ", &saveptr);
  if(protocol == NULL) {
    strncpy(log_message, "Failure: Parsing HTTP Status-Line", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }

  /* Parse the Status-Code. */
  code = strtok_r(saveptr, " ", &saveptr);
  if(code == NULL) {
    strncpy(log_message, "Failure: Parsing HTTP Status-Line", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }

  /* Parse the Reason-Phrase. */
  reason = strtok_r(saveptr, "\r\n", &saveptr);
  if(reason == NULL) {
    strncpy(log_message, "Failure: Parsing HTTP Status-Line", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }

  /* Save the HTTP-Version into the structure. */
  status_line->http_protocol = (char *) malloc(strlen(protocol));
  if (status_line->http_protocol == NULL) {
    strncpy(log_message, "Failure: malloc() http_protocol", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }
  strcpy(status_line->http_protocol, protocol);

  /* Save the Status-Code into the structure. */
  status_line->status_code = (char *) malloc(strlen(code));
  if (status_line->status_code == NULL) {
    strncpy(log_message, "Failure: malloc() status_code", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }
  strcpy(status_line->status_code, code);

  /* Save the Reason-Phrase into the structure. */
  status_line->reason_phrase = (char *) malloc(strlen(reason));
  if (status_line->reason_phrase == NULL) {
    strncpy(log_message, "Failure: malloc() reason_phrase", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }
  strcpy(status_line->reason_phrase, reason);
  return;
}
/* -------------------------------------------------------------------------------------------------------*/
/* Parse HTTP request method line */
void parse_method(struct method_line *method_line, char *http_line){
  char log_message[LOG_SIZE];
  char *saveptr = http_line;
  char *name;
  char *destination;
  char *protocol;

  /* Parse the method name. */
  name = strtok_r(saveptr, " ", &saveptr);
  if(name == NULL) {
    strncpy(log_message, "Failure: Parsing HTTP Method", LOG_SIZE);
    log_event(log_message);
    // respond('400 BAD REQUEST');
    pthread_exit(NULL);
  }

  /* Parse the destination URI. */
  destination = strtok_r(saveptr, " ", &saveptr);
  if(name == NULL) {
    strncpy(log_message, "Failure: Parsing HTTP Method", LOG_SIZE);
    log_event(log_message);
    // respond('400 BAD REQUEST');
    pthread_exit(NULL);
  }

  /* Parse the Protocol. */
  protocol = strtok_r(saveptr, " ", &saveptr);
  if(name == NULL) {
    strncpy(log_message, "Failure: Parsing HTTP Method", LOG_SIZE);
    log_event(log_message);
    // respond('400 BAD REQUEST');
    pthread_exit(NULL);
  }

  /* Save the method type into the structure. */
  method_line->method_type = (char *) malloc(strlen(name));
  if (method_line->method_type == NULL) {
    strncpy(log_message, "Failure: malloc() method_type", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }
  strcpy(method_line->method_type, name);

  /* Save the destination uri into the structure. */
  method_line->destination_uri = (char *) malloc(strlen(destination));
  if (method_line->destination_uri== NULL) {
    strncpy(log_message, "Failure: malloc() destination_uri", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }
  //TODO: Further parse destination_uri, possibly introduce another struct for it.
  strcpy(method_line->destination_uri, destination);

  /* Save the http protocol into the structure. */
  method_line->http_protocol = (char *) malloc(strlen(protocol));
  if (method_line->http_protocol == NULL) {
    strncpy(log_message, "Failure: malloc() destination_uri", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }
  strcpy(method_line->http_protocol, protocol);

  return;
}
/* -------------------------------------------------------------------------------------------------------*/

void parse_header(struct header_array *headers, char *header_line) {
  char *saveptr = header_line;
  char log_message[LOG_SIZE];
  char* name;
  int save_len;

  /* Parse the header. */
  name = strtok_r(saveptr, ":", &saveptr);
  if(name == NULL) {
    strncpy(log_message, "Failure: Parsing HTTP Header", LOG_SIZE);
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

      /* Trim leading whitespace on header value. */
      // NOTE: This approach eliminates the odd behavior when trimming the space, but perhaps there is a more elegant way of doing it?
      save_len = strlen(saveptr);
      saveptr++;

      // headers[i].header_value = (char *) malloc(strlen(saveptr));
      headers[i].header_value = (char *) malloc(save_len);
      if (headers[i].header_value == NULL) {
        strncpy(log_message, "Failure: malloc() header_value", LOG_SIZE);
        log_event(log_message);
        pthread_exit(NULL);
      }
      strcpy(headers[i].header_value, saveptr);

      break;
    }
  }
  return;
}
/* -------------------------------------------------------------------------------------------------------*/
/* Checks for the presence of Content-Length HTTP header in a parsed message */
void check_content_length(struct message_t *http_message){
  char log_message[LOG_SIZE];
  char *cl_conversion;

  /* Iterate through headers, check for Content-Length */
  for(int i = 0; i < HEADER_ARRAY_LENGTH; i++) {

    if(http_message->headers[i].header_name != NULL && http_message->headers[i].header_value != NULL) {

      /* We do not support Transfer-Encoding */
      if (strcmp(http_message->headers[i].header_name, "Transfer-Encoding") == 0) {
        strncpy(log_message, "Failure: Transfer-Encoding Header.", LOG_SIZE);
        // respond("501 (Not Implemented)");
        log_event(log_message);
        pthread_exit(NULL);
      }

      /* Content-Length's presence indicates an HTTP response or certain requests. */
      if (strcmp(http_message->headers[i].header_name, "Content-Length") == 0) {

        /* If the message is indeed a request, only POST requests will have the method line parsed. */
        /* If both these fields are NULL, it means a non-supported request method was sent. */
        if(http_message->request_method_info.method_type == NULL && http_message->response_status_line.http_protocol == NULL) {
            strncpy(log_message, "Failure: Unsupported Method.", LOG_SIZE);
            // respond("501 (Not Implemented)");
            log_event(log_message);
            pthread_exit(NULL);
        }

        http_message->data_type.is_response = 0;

        /* Reset errno to 0 before strtol call */
        errno = 0;

        /* Save content_length. */
        http_message->data_type.content_length = strtol(http_message->headers[i].header_value, &cl_conversion, 10);

        /* If errno is set, the 0 returned is invalid. */
        if (http_message->data_type.content_length == 0 && errno != 0){
          strncpy(log_message, "Failure: Could not convert Content-Length", LOG_SIZE);
          log_event(log_message);
          pthread_exit(NULL);
        }
      }
     }
   }
   return;
}
