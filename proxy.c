/* Functionality of Proxy web server. */
#include "proxy.h"
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>

//Function Prototypes
static void read_data(int client, char **request_buffer, char *body_beginning);
static void read_body(int client, char *body_buffer, char *body_overflow, long content_length);
static int connect_to_host(void);
static int send_request_to_host(void);
static int get_response(void);

//Functions
/* -------------------------------------------------------------------------------------------------------*/
/* Read HTTP request/response headers */
static void read_data(int client, char **request_buffer, char *body_beginning){

  char log_message[LOG_SIZE];
  int n;
  int read_length = 0;
  char *needle;

  while (0 == 0) {

    n = read(client, *request_buffer + read_length, READ_SIZE);

    if(n == -1) {
      strncpy(log_message, "Failure: Read HTTP Request", LOG_SIZE);
      log_event(log_message);
      free(*request_buffer);
      pthread_exit(NULL);
    }

    /* Keep track of current position */
    read_length += n;

    /* Check for the end of the HTTP request. */
    /* Marked by an empty line preceding a CRLF ("\n\r\n"). */
    needle = strstr(*request_buffer, "\n\r\n");
    if (needle != NULL) {

      /* We bypass the "\n\r\n" needle points to, and copy beginning of the message body */
      strcpy(body_beginning, needle + 3);

      /* Add terminating null after the "\n\r\n" occurs */
      strcpy(*request_buffer + read_length - strlen(body_beginning), "\0");

      //NOTE: The strcpy logic appears to be correct, based on testing. However, check back here if issues arise later.

      break;
    }

    /* Realloc enough memory for next read() */
    *request_buffer = realloc(*request_buffer, strlen(*request_buffer) + READ_SIZE);
    if (*request_buffer == NULL) {
      strncpy(log_message, "Failure: realloc() request_buffer", LOG_SIZE);
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

  if (-1 == (n = read(client, body_buffer + overflow_size, content_length))) {
    strncpy(log_message, "Failure: Read HTTP Body", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }

  body_buffer[n + overflow_size] = 0;

  //NOTE: Use this to print non-null terminated string. In case having the null becomes an issue.
  // fwrite(body_buffer, sizeof(char), n + overflow_size, stdout);

  return;
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
/* Authenticate access to admin functions, and fulfill them. */
void authenticate(){

  //TODO: Check if its a stop request, currently assumes any admin request is a stop.

  /* Send signal to stop server, using global variable master_pid */
  kill(master_pid, SIGUSR1);

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
  char log_message[LOG_SIZE];
  struct request_t http_request;
  http_request.data_type.is_response = 1; //Default to request.
  char body_overflow[READ_SIZE];
  char *request_buffer = NULL;

  request_buffer = (char *) malloc(READ_SIZE);
  if (request_buffer == NULL) {
    strncpy(log_message, "Failure: malloc() request_buffer", LOG_SIZE);
    log_event(log_message);
    pthread_exit(NULL);
  }

  read_data(client, &request_buffer, body_overflow);
  printf("Body Overflow:%s\n", body_overflow);
  parse_request(client, request_buffer, &http_request);

  /* Store the content_length-sized message body, plus a terminating null. */
  char body_buffer[http_request.data_type.content_length + 1];

  /* Read message body if the data is a response (or POST request) */
  if(http_request.data_type.is_response == 0) {

    read_body(client, body_buffer, body_overflow, http_request.data_type.content_length);
    printf("Entire Body:%s\n", body_buffer);
  }

  char http_message[strlen(request_buffer) + strlen(body_buffer)];

  strcpy(http_message, request_buffer);
  if (http_request.data_type.is_response == 0) {
    strcat(http_message, body_buffer);
  }

  printf("Full HTTP Message:\n%s\n", http_message);

  char request_buffer_final[strlen(http_message) + 100];

  /* Form sample response */
  strcpy(request_buffer_final,"HTTP/1.x 200 OK\nContent-Type: text/html\n\n" );
  strcat(request_buffer_final, http_message);

  respond(client, request_buffer_final);

  authenticate();

  /* Free all malloc()'d memory */
  free(request_buffer);
  free(http_request.method_info.method_type);
  free(http_request.method_info.destination_uri);
  free(http_request.method_info.http_protocol);

  /* Only free malloc'd indeces */
  for(int i = 0; i < HEADER_ARRAY_LENGTH; i++) {
    if(http_request.headers[i].header_name != NULL && http_request.headers[i].header_value != NULL)
    {
      free(http_request.headers[i].header_name);
      free(http_request.headers[i].header_value);
    }
   }

  pthread_exit(NULL);
}
