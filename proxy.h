#ifndef PROXY_H_INCLUDED
#define PROXY_H_INCLUDED

/* Function Prototypes */
// int start_server(int port);
// int accept_connection(int server);
void *serve_request(void *thread_info);
// int read_request(void);
// int parse_request(void);
// int authenticate(void);
// int connect_to_host(void);
// int send_request_to_host(void);
// int get_response(void);
// int respond(void);
void log_event(char *log_message);

/* Constants */
extern int master_pid;
static const int LOG_SIZE = 200;
static const int REQUEST_SIZE = 4096;

/* Preprocessing */
#include <pthread.h>
#include <fcntl.h>
#include <strings.h>
#include <signal.h>
#endif
