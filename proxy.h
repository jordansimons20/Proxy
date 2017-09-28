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

int log_result(void); //Will likely be a void function, accepting a string message.
extern int master_pid;

#include <pthread.h>
#endif
