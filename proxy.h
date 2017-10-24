#ifndef PROXY_H_INCLUDED
#define PROXY_H_INCLUDED
/* Constants */
extern int master_pid;
static const int LOG_SIZE = 200;
// static const int REQUEST_SIZE = 65536;
static const int REQUEST_SIZE = 8192;
static const int HTTP_HEADER_NAME_SIZE = 120;
static const int HEADER_ARRAY_LENGTH = 100; // Maximum allowed number of headers in a request.
static const int READ_SIZE = 500;

/* For each HTTP header */
struct header_array {
  char *header_name;
  char *header_value;
};

/* For each HTTP method line */
struct method_line{
  char *method_type;
  char *destination_uri;
  char *http_protocol;
};

/* Contains if HTTP message is request or response */
struct http_type {
  int is_response;
  long content_length; // Responses/POST requests only.
};

/* Structure for every HTTP request/response */
struct request_t {
  struct method_line method_info;
  struct header_array headers[HEADER_ARRAY_LENGTH];
  struct http_type data_type;
};

/* Function Prototypes */
void *serve_request(void *thread_info);
int parse_message(int client, char *request_buffer, struct request_t *http_request);
void authenticate();
void respond(int client, char *content);
void log_event(char *log_message);
int stop_server(int server);
void parse_method(struct method_line *method_line, char *http_line);
void parse_header(struct header_array *headers, char *header_line);

/* Preprocessing */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <strings.h>
#include <signal.h>
#endif
