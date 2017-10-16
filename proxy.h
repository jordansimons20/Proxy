#ifndef PROXY_H_INCLUDED
#define PROXY_H_INCLUDED

/* Structure for every HTTP request/response */
struct request_t {
  char *header_name;
  char *header_value;
};

/* Function Prototypes */
// int start_server(int port);
// int accept_connection(int server);
void *serve_request(void *thread_info);
// int read_request(void);
int parse_request(int client, char *request_buffer, struct request_t *header_array);
void authenticate();
// int connect_to_host(void);
// int send_request_to_host(void);
// int get_response(void);
void respond(int client, char *content);
void log_event(char *log_message);
int stop_server(int server);
void parse_method();
void parse_header(struct request_t *headers, char *header_line);

/* Constants */
extern int master_pid;
static const int LOG_SIZE = 200;
// static const int REQUEST_SIZE = 65536;
static const int REQUEST_SIZE = 8192;
static const int HTTP_HEADER_NAME_SIZE = 120;

/* Preprocessing */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <strings.h>
#include <signal.h>
#endif
