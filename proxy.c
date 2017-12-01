/* Functionality of Proxy web server. */
#include "proxy.h"
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>


//Function Prototypes
static int read_data(int client, char **header_buffer, char *body_beginning);
static int read_body(int client, char **body_buffer, char *body_overflow, long content_length, int is_response, int body_length);
static int connect_to_host(struct message_t *http_request);
static void send_request_to_host(int host_socket, char *http_request, int message_length);

//Functions
/* -------------------------------------------------------------------------------------------------------*/
/* Read HTTP request/response headers */
static int read_data(int client, char **header_buffer, char *body_beginning){

  char log_message[LOG_SIZE];
  int n;
  int body_beginning_length;
  int read_length = 0;
  char *needle = NULL;

  while (0 == 0) {

    n = read(client, *header_buffer + read_length, READ_SIZE);

    if(n == -1) {
      strncpy(log_message, "Failure: Read HTTP Request", LOG_SIZE);
      log_event(log_message);
      //free(*header_buffer);
      pthread_exit(NULL);
    }

    /* Keep track of current position */
    read_length += n;

    /* Check for the end of the HTTP request. */
    /* Marked by an empty line preceding a CRLF ("\n\r\n"). */
    needle = strstr(*header_buffer, "\n\r\n");
    if (needle != NULL) {

      /* We bypass the "\n\r\n" needle points to, and copy beginning of the message body */

      body_beginning_length = read_length - (3 -(*header_buffer - needle));
      printf("body_beginning_length: %d\n", body_beginning_length);

      memcpy(body_beginning, needle + 3, body_beginning_length);

      /* Add terminating null after the "\n\r\n" occurs */
      *(needle + 3) = 0;

      break;
    }

    /* Realloc enough memory for next read() */
    *header_buffer = realloc(*header_buffer, read_length + READ_SIZE);
    if (*header_buffer == NULL) {
      strncpy(log_message, "Failure: realloc() header_buffer", LOG_SIZE);
      log_event(log_message);
      pthread_exit(NULL);
    }
  }

  return body_beginning_length;
}
/* -------------------------------------------------------------------------------------------------------*/
/* Read HTTP message body. */
static int read_body(int client, char **body_buffer, char *body_overflow, long content_length, int is_response, int body_length) {

  char log_message[LOG_SIZE];
  int n;

  memcpy(*body_buffer, body_overflow, body_length);

  /* Subtract data already read in read_data() */
  content_length -= body_length;

  /* For requests, we know content_length is accurate. */
  if(is_response == 1) {
    puts("Request Implementation");
    if (-1 == (n = read(client, *body_buffer + body_length, content_length ))) {
      strncpy(log_message, "Failure: Read HTTP Body", LOG_SIZE);
      //perror("Body");
      log_event(log_message);
      pthread_exit(NULL);
    }

    body_length += n;
    (*body_buffer)[body_length] = 0;
    printf("Bytes Read: %d\n", n);

  }

  /* For responses, we read until there is no more data. */
  else {
    puts("Response Implementation");
    while (0 == 0) {

      puts("About to read");
      n = recv(client, *body_buffer + body_length, content_length, MSG_WAITALL);

      printf("n: %d\n", n);

      if(n == -1) {
        strncpy(log_message, "Failure: Read HTTP Message Body", LOG_SIZE);
        log_event(log_message);
        perror("Response Read Body");
        pthread_exit(NULL);
      }

      /* Keep track of current position */
      body_length += n;

      /* No more data to read. */
      if(n < content_length || (n == 0 && content_length == 0)){
        break;
      }

      /* Realloc enough memory for next read() */
      *body_buffer = realloc(*body_buffer, body_length + content_length + 1);
      if (*body_buffer == NULL) {
        strncpy(log_message, "Failure: realloc() header_buffer", LOG_SIZE);
        log_event(log_message);
        pthread_exit(NULL);
      }
    }
    (*body_buffer)[body_length] = 0;
    printf("Bytes Read: %d\n", body_length);
  }

  return body_length;
}
/* -------------------------------------------------------------------------------------------------------*/
/* Connects to the host specified in the http_request, returns the host socket. */
static int connect_to_host(struct message_t *http_request){
  char log_message[LOG_SIZE];
  struct addrinfo hints, *servinfo, *p;
  int rv, host_socket;
  char *host = http_request->request_method_info.destination_uri.host;
  char *port = http_request->request_method_info.destination_uri.port;

  /* Setup the hints parameter for getaddrinfo() */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC; // Allow IPv4 and IPv6
  hints.ai_socktype = SOCK_STREAM;

  /* Get information to connect to the host. */
  if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
    strncpy(log_message, "Failure: getaddrinfo", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }

  /* Loop through all the results and connect to the first we can */
  for(p = servinfo; p != NULL; p = p->ai_next) {
      if ((host_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
        continue;
      }

      if (connect(host_socket, p->ai_addr, p->ai_addrlen) == -1) {
        close(host_socket);
        continue;
      }

      break; /* If we get here, we must have connected successfully */
  }

  if (p == NULL) {
      /* Looped off the end of the list with no connection */
      strncpy(log_message, "Failure: could not connect to host", LOG_SIZE);
      log_event(log_message);
      pthread_exit(NULL);
  }

  freeaddrinfo(servinfo);
  puts("Connected!");

  return host_socket;
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
/* Send the modified HTTP request to the host */
static void send_request_to_host(int host_socket, char *http_request, int message_length){
  char log_message[LOG_SIZE];

  if( write(host_socket, http_request, message_length) == -1 ) {
    strncpy(log_message, "Failure: write to host", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }
  fsync(host_socket);

  puts("Sent request to host.");

  return;
}
/* -------------------------------------------------------------------------------------------------------*/
/* Responds to client with given content. */
void respond(int client, char *content, int response_length){

  char log_message[LOG_SIZE];
	if (-1 == write(client, content, response_length)) {
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
  int host;
  char log_message[LOG_SIZE];
  struct message_t http_request;
  http_request.data_type.is_response = 1;
  http_request.data_type.has_body = 1;
  struct message_t http_response;
  http_response.data_type.is_response = 0;
  http_response.data_type.has_body = 1;
  char body_overflow[READ_SIZE];
  char *header_buffer = NULL;
  char *response_header_buffer = NULL;
  char *response_body_buffer = NULL;
  int body_length;

  /* Initialize struct values to NULL */
  for(int i = 0; i < HEADER_ARRAY_LENGTH; i++) {
      http_request.headers[i].header_name = NULL;
      http_request.headers[i].header_value = NULL;
      http_response.headers[i].header_name = NULL;
      http_response.headers[i].header_value = NULL;
   }
   http_request.request_method_info.method_type = NULL;
   http_request.request_method_info.http_protocol = NULL;
   http_request.request_method_info.relative_method_line = NULL;
   http_request.request_method_info.destination_uri.original_destination_uri = NULL;
   http_request.request_method_info.destination_uri.host = NULL;
   http_request.request_method_info.destination_uri.port = NULL;
   http_request.request_method_info.destination_uri.absolute_path = NULL;
   http_request.response_status_line.http_protocol = NULL;
   http_request.response_status_line.status_code = NULL;
   http_request.response_status_line.reason_phrase = NULL;

   http_response.request_method_info.method_type = NULL;
   http_response.request_method_info.http_protocol = NULL;
   http_response.request_method_info.relative_method_line = NULL;
   http_response.request_method_info.destination_uri.original_destination_uri = NULL;
   http_response.request_method_info.destination_uri.host = NULL;
   http_response.request_method_info.destination_uri.port = NULL;
   http_response.request_method_info.destination_uri.absolute_path = NULL;
   http_response.response_status_line.http_protocol = NULL;
   http_response.response_status_line.status_code = NULL;
   http_response.response_status_line.reason_phrase = NULL;

  header_buffer = malloc(READ_SIZE);
  if (header_buffer == NULL) {
    strncpy(log_message, "Failure: malloc() header_buffer", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }

  body_length = read_data(client, &header_buffer, body_overflow);
  parse_message(header_buffer, &http_request);

  /* Store the content_length-sized message body, plus a terminating null. */
  printf("HTPP Request content_length: %ld\n", http_request.data_type.content_length);
  char body_buffer[http_request.data_type.content_length + 1];
  char *body_buffer_pointer = body_buffer;

  /* Read message body if it exists */
  if(http_request.data_type.has_body == 0) {
    body_length = read_body(client, &body_buffer_pointer, body_overflow, http_request.data_type.content_length, http_request.data_type.is_response, body_length);
  }

  /* Replace the method-line with the relative version. */
  char relative_header_buffer[strlen(header_buffer)];
  strcpy(relative_header_buffer, http_request.request_method_info.relative_method_line);

  /* Add the rest of the headers after the original method_line */
  strcat(relative_header_buffer, header_buffer + http_request.request_method_info.original_length);

  int relative_len = strlen(relative_header_buffer);

  char http_message[relative_len + body_length];
  strcpy(http_message, relative_header_buffer);

  /* Add the body, if there is one. */
  if (http_request.data_type.has_body == 0) {
    memcpy(http_message + relative_len, body_buffer, body_length);
  }

  body_length += relative_len;

  printf("HTTP Request:\n%s", http_message);

  host = connect_to_host(&http_request);

  send_request_to_host(host, http_message, body_length);

  /* We read the response the same way as the initial request */
  response_header_buffer = malloc(READ_SIZE);
  if (response_header_buffer == NULL) {
    strncpy(log_message, "Failure: malloc() response_header_buffer", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }

  /* Clear body_overflow for re-use */
  memset(&body_overflow[0], 0, sizeof(body_overflow));

  body_length = read_data(host, &response_header_buffer, body_overflow);

  parse_message(response_header_buffer, &http_response);

  response_body_buffer = malloc( (http_response.data_type.content_length + 1));
  if (response_body_buffer == NULL) {
    strncpy(log_message, "Failure: malloc() response_body_buffer", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }

  if (http_response.data_type.content_length != 0) {// TODO: We check this twice (also in read_body(), remove one)
  body_length = read_body(host, &response_body_buffer, body_overflow, http_response.data_type.content_length, http_response.data_type.is_response, body_length);
  }

  close(host);

  int response_header_length = strlen(response_header_buffer);

  char full_http_response[response_header_length + body_length];
  strcpy(full_http_response, response_header_buffer);

  /* Add the body, if there is one. */
  if (http_response.data_type.content_length != 0) {
    memcpy(full_http_response + strlen(full_http_response), response_body_buffer, body_length);
  }

  body_length += response_header_length;

  fwrite(full_http_response, sizeof(char), body_length, stdout);

  respond(client, full_http_response, body_length);

  /* Free all malloc()'d memory */
  free(header_buffer);
  free(response_header_buffer);
  free(response_body_buffer);
  free(http_request.request_method_info.method_type);
  free(http_request.request_method_info.destination_uri.original_destination_uri);
  free(http_request.request_method_info.relative_method_line);
  free(http_request.request_method_info.http_protocol);
  free(http_request.request_method_info.destination_uri.absolute_path);
  free(http_request.request_method_info.destination_uri.host);
  free(http_request.request_method_info.destination_uri.port);

  /* Only free malloc'd indeces */
  for(int i = 0; i < HEADER_ARRAY_LENGTH; i++) {
    if(http_request.headers[i].header_name != NULL && http_request.headers[i].header_value != NULL)
    {
      free(http_request.headers[i].header_name);
      free(http_request.headers[i].header_value);
    }
   }

  free(http_response.response_status_line.http_protocol);
  free(http_response.response_status_line.status_code);
  free(http_response.response_status_line.reason_phrase);

  /* Only free malloc'd indeces */
  for(int i = 0; i < HEADER_ARRAY_LENGTH; i++) {
    if(http_response.headers[i].header_name != NULL && http_response.headers[i].header_value != NULL)
    {
      free(http_response.headers[i].header_name);
      free(http_response.headers[i].header_value);
    }
   }

  fsync(client);
  close(client);
  // authenticate();

  pthread_exit(NULL);
}
