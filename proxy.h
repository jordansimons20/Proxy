#ifndef PROXY_H_INCLUDED
#define PROXY_H_INCLUDED
/* Constants */
extern int master_pid;
static const int LOG_SIZE = 200;
static const int REQUEST_SIZE = 8192;
static const int HTTP_HEADER_NAME_SIZE = 120;
static const int HEADER_ARRAY_LENGTH = 100; // Maximum allowed number of headers in a request.
static const int READ_SIZE = 500;

/* For each HTTP header */
struct header_array {
  char *header_name;
  char *header_value;
};

/* Contains pieces of the destination_uri in HTTP requests */
struct destination_uri {
  char *original_destination_uri;
  char *host;
  char *port;
  char *absolute_path;
};
/* For each HTTP request method line. Each value is NULL if the HTTP message is a response. */
struct method_line{
  char *method_type;
  struct destination_uri destination_uri;
  char *http_protocol;
  int original_length;
  char *relative_method_line;
};

/* For each HTTP response status line. Each value is NULL if the HTTP message is a request. */
struct status_line{
  char *http_protocol;
  char *status_code;
  char *reason_phrase;
};

/* Contains if HTTP message is request or response */
struct http_type {
  int is_response;
  long content_length; // Responses/POST requests only.
};

/* Structure for every HTTP request/response */
struct message_t {
  struct method_line request_method_info; //Requests only
  struct status_line response_status_line; //Responses only
  struct header_array headers[HEADER_ARRAY_LENGTH];
  struct http_type data_type;
};

/* Function Prototypes */
void *serve_request(void *thread_info);
int parse_message(int client, char *request_buffer, struct message_t *http_request);
void authenticate();
void respond(int client, char *content);
void log_event(char *log_message);
int stop_server(int server);
void parse_status_line(struct status_line *status_line, char *http_line);
void parse_method(struct method_line *method_line, char *http_line);
void parse_header(struct header_array *headers, char *header_line);
void check_content_length(struct message_t *http_message);

/* Preprocessing */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <strings.h>
#include <signal.h>
#endif
