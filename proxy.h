#ifndef PROXY_H_INCLUDED
#define PROXY_H_INCLUDED
/* Constants */
extern int master_pid;
static const int LOG_SIZE = 200;
// static const int REQUEST_SIZE = 65536;
static const int REQUEST_SIZE = 8192;
static const int HTTP_HEADER_NAME_SIZE = 120;
static const int HEADER_ARRAY_LENGTH = 100; // Maximum allowed number of headers in a request.

/* For each HTTP header */
struct header_array {
  char *header_name;
  char *header_value;
};

/* Structure for every HTTP request/response */
struct request_t {
  char *method;
  struct header_array headers[HEADER_ARRAY_LENGTH];
};

/* Function Prototypes */
void *serve_request(void *thread_info);
int parse_request(int client, char *request_buffer, struct request_t *http_request);
void authenticate();
void respond(int client, char *content);
void log_event(char *log_message);
int stop_server(int server);
void parse_method(char *method, char *method_line);
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
