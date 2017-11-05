/* Methods called in http.l to further parse HTTP */
#include "proxy.h"
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
//Functions
static void parse_destination_uri(char *destination_uri, struct method_line *method_line);
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
/* Retrieve destination host name, specified port (if any), and absolute path */
static void parse_destination_uri(char *destination_uri, struct method_line *method_line){
  /* Make copy of destination_uri for destructive parsing */
  struct servent *servent;
  char destination_uri_copy[strlen(destination_uri)];
  strcpy(destination_uri_copy, destination_uri);
  char *saveptr = destination_uri_copy;
  char log_message[LOG_SIZE];
  char *parsed_host;
  char *final_host;
  char *parsed_path;
  int port;
  char *port_conversion;
  char final_path[strlen(destination_uri)]; //Cannot exceed this length

  /* Move passed the initial http:// */
  if( strtok_r(saveptr, "/", &saveptr) == NULL) {
    strncpy(log_message, "Failure: Parsing destination_uri", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }
  saveptr = saveptr + 1;

  /* Port and Absolute Path may or may not be specified: www.example.com:8080/path/ (specified) www.example.com:8080/ (unspecified)  */
  parsed_host = strtok_r(saveptr, "/", &saveptr);
  if(parsed_host == NULL) {
    strncpy(log_message, "Failure: Parsing host destination_uri", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }

  /* The absolute path may or may not exist. */
  parsed_path = strtok_r(saveptr, " ", &saveptr);

  /* If no absolute path is specified, we use "/" */
  if(parsed_path == NULL) {
    strcpy(final_path, "/");
  }

  /* Otherwise, use the specified path */
  else {
    /* The absolute path has its first '/' cut off by the parsing */
    strcpy(final_path, "/");
    strcat(final_path, parsed_path);
  }

  /* We don't know if a port has been specified, so we check host */
  final_host = strtok_r(parsed_host, ":", &parsed_host);
  if(final_host == NULL) {
    strncpy(log_message, "Failure: Parsing host destination_uri", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }
  /* NULL parsed_host indicates no port specified, so we use the default */
  if(parsed_host == NULL) {
    servent = getservbyname("http", NULL);
    endservent();

    if(servent == NULL) {
      strncpy(log_message, "Failure: Could not find standard port", LOG_SIZE);
      log_event(log_message);
      pthread_exit(NULL);
    }

    /* Convert port to host byte order */
    port = ntohs(servent->s_port);

  }

  else {
    /* Convert the specified port to an integer */
    port = strtol(parsed_host, &port_conversion, 10);

    /* Check for error */
    if (port == 0){
      strncpy(log_message, "Failure: Could not convert Port", LOG_SIZE);
      log_event(log_message);
      pthread_exit(NULL);
    }
  }

  /* Save the Host into the structure. */
  method_line->destination_uri.host = (char *) malloc(strlen(final_host));
  if (method_line->destination_uri.host == NULL) {
    strncpy(log_message, "Failure: malloc() host", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }
  strcpy(method_line->destination_uri.host, final_host);

  /* Save the absolute path into the structure. */
  method_line->destination_uri.absolute_path = (char *) malloc(strlen(final_path));
  if (method_line->destination_uri.absolute_path == NULL) {
    strncpy(log_message, "Failure: malloc() host", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }
  strcpy(method_line->destination_uri.absolute_path, final_path);

  /* Save the port into the structure. */
  method_line->destination_uri.port = port;

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

  /* Save the original length */
  method_line->original_length = strlen(http_line);

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

  /* Further parse the destination URI substring */
  parse_destination_uri(destination, method_line);

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

  /* Save the original destination uri into the structure. */
  method_line->destination_uri.original_destination_uri = (char *) malloc(strlen(destination));
  if (method_line->destination_uri.original_destination_uri== NULL) {
    strncpy(log_message, "Failure: malloc() original_destination_uri", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }
  strcpy(method_line->destination_uri.original_destination_uri, destination);

  /* Save the http protocol into the structure. */
  method_line->http_protocol = (char *) malloc(strlen(protocol));
  if (method_line->http_protocol == NULL) {
    strncpy(log_message, "Failure: malloc() http_protocol", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }
  strcpy(method_line->http_protocol, protocol);

  /* Save the crafted relative request line into the structure. */
  method_line->relative_method_line = (char *) malloc(strlen(destination));
  if (method_line->relative_method_line == NULL) {
    strncpy(log_message, "Failure: malloc() relative_method_line", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }
  /* Format: Method SP Request-URI SP HTTP-Version CRLF */
  strcpy(method_line->relative_method_line, method_line->method_type );
  strcat(method_line->relative_method_line, " ");
  strcat(method_line->relative_method_line, method_line->destination_uri.absolute_path);
  strcat(method_line->relative_method_line, " ");
  strcat(method_line->relative_method_line, method_line->http_protocol);
  // NOTE: method_line->http_protocol contains the traling CRLF

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
