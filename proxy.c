/* Functionality of Proxy web server. */
#include "proxy.h"
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>


//Function Prototypes
static void read_data(int client, char **header_buffer, char *body_beginning);
static void read_body(int client, char *body_buffer, char *body_overflow, long content_length);
static int connect_to_host(struct message_t *http_request);
static void send_request_to_host(int host_socket, char *http_request);

//Functions
/* -------------------------------------------------------------------------------------------------------*/
/* Read HTTP request/response headers */
static void read_data(int client, char **header_buffer, char *body_beginning){

  char log_message[LOG_SIZE];
  int n;
  int read_length = 0;
  char *needle;

  while (0 == 0) {

    n = read(client, *header_buffer + read_length, READ_SIZE);

    if(n == -1) {
      strncpy(log_message, "Failure: Read HTTP Request", LOG_SIZE);
      log_event(log_message);
      free(*header_buffer);
      pthread_exit(NULL);
    }

    /* Keep track of current position */
    read_length += n;

    /* Check for the end of the HTTP request. */
    /* Marked by an empty line preceding a CRLF ("\n\r\n"). */
    needle = strstr(*header_buffer, "\n\r\n");
    if (needle != NULL) {

      /* We bypass the "\n\r\n" needle points to, and copy beginning of the message body */
      strcpy(body_beginning, needle + 3);

      /* Add terminating null after the "\n\r\n" occurs */
      *(needle + 3) = 0;

      break;
      //NOTE: return here instead? break makes another realloc()? Investigate
    }

    /* Realloc enough memory for next read() */
    *header_buffer = realloc(*header_buffer, strlen(*header_buffer) + READ_SIZE);
    if (*header_buffer == NULL) {
      strncpy(log_message, "Failure: realloc() header_buffer", LOG_SIZE);
      log_event(log_message);
      pthread_exit(NULL);
    }
  }

  return;
}
/* -------------------------------------------------------------------------------------------------------*/
/* Read HTTP message body. */
static void read_body(int client, char *body_buffer, char *body_overflow, long content_length) {

  char log_message[LOG_SIZE];
  int n;
  int overflow_size = strlen(body_overflow);

  strcpy(body_buffer, body_overflow);

  /* Subtract data already read in read_data() */
  content_length -= overflow_size;

  // printf("content length: %d\n", content_length);

  if (-1 == (n = read(client, body_buffer + overflow_size, content_length * 2))) {
    strncpy(log_message, "Failure: Read HTTP Body", LOG_SIZE);
    //perror("Body");
    log_event(log_message);
    pthread_exit(NULL);
  }

  // printf("Bytes Read: %lu\n", n);

  body_buffer[n + overflow_size] = 0;

  //NOTE: Use this to print non-null terminated string. In case having the null becomes an issue.
  // fwrite(body_buffer, sizeof(char), n + overflow_size, stdout);

  return;
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

  freeaddrinfo(servinfo); /* All done with this structure */
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
static void send_request_to_host(int host_socket, char *http_request){
  char log_message[LOG_SIZE];

  if( write(host_socket, http_request, strlen(http_request)) == -1 ) {
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
void respond(int client, char *content){

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
  int host;
  char log_message[LOG_SIZE];
  struct message_t http_request;
  http_request.data_type.is_response = 1;
  struct message_t http_response;
  http_response.data_type.is_response = 0;
  char body_overflow[READ_SIZE];
  char *header_buffer = NULL;
  char *response_header_buffer = NULL;
  char *response_body_buffer = NULL;

  header_buffer = (char *) malloc(READ_SIZE);
  if (header_buffer == NULL) {
    strncpy(log_message, "Failure: malloc() header_buffer", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }

  read_data(client, &header_buffer, body_overflow);
  parse_message(header_buffer, &http_request);

  /* Store the content_length-sized message body, plus a terminating null. */
  char body_buffer[http_request.data_type.content_length + 1];

  /* Read message body if the data is a response (or POST request) */
  if(http_request.data_type.is_response == 0) {
    read_body(client, body_buffer, body_overflow, http_request.data_type.content_length);
  }

  /* Replace the method-line with the relative version. */
  char relative_header_buffer[strlen(header_buffer)];
  strcpy(relative_header_buffer, http_request.request_method_info.relative_method_line);

  /* Add the rest of the headers after the original method_line */
  strcat(relative_header_buffer, header_buffer + http_request.request_method_info.original_length);

  char http_message[strlen(relative_header_buffer) + strlen(body_buffer)];
  strcpy(http_message, relative_header_buffer);

  /* Add the body, if there is one. */
  if (http_request.data_type.is_response == 0) {
    strcat(http_message, body_buffer);
  }

  printf("HTTP Request:\n%s", http_message);

  char header_buffer_final[strlen(http_message) + 100];

  /* Form sample response */
  strcpy(header_buffer_final,"HTTP/1.x 200 OK\nContent-Type: text/html\n\n" );
  strcat(header_buffer_final, http_message);

  respond(client, header_buffer_final);

  host = connect_to_host(&http_request);

  send_request_to_host(host, http_message);

  /* We read the response the same way as the initial request */
  response_header_buffer = (char *) malloc(READ_SIZE);
  if (response_header_buffer == NULL) {
    strncpy(log_message, "Failure: malloc() response_header_buffer", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }

  /* Clear body_overflow for re-use */
  memset(&body_overflow[0], 0, sizeof(body_overflow));


  read_data(host, &response_header_buffer, body_overflow);
  printf("Response Headers:\n%s", response_header_buffer);

  puts("End of Response Headers.");

  parse_message(response_header_buffer, &http_response);

  //TODO: Read body.
  response_body_buffer = (char *) malloc(http_response.data_type.content_length + 1);
  if (response_body_buffer == NULL) {
    strncpy(log_message, "Failure: malloc() response_body_buffer", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }

  read_body(host, response_body_buffer, body_overflow, http_response.data_type.content_length);

  printf("Response body: %s \n", response_body_buffer);

  close(host);

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

  //TODO: free() memory from http_response structure.

   authenticate();
   close(client);

  pthread_exit(NULL);
}
