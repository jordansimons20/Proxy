%{
#include <stdio.h>
#include "proxy.h"
#include <errno.h>
  /*
    flex -i -I http.l
    cc -c -o lex.yy.o lex.yy.c
    cc -o foobar all_your_o_files lex.yy.o -lfl
  */
%}

%option reentrant
%option extra-type="struct message_t*"
%%
%{
  struct message_t *http_request = yyextra;
%}
^"HTTP/"[^ ]+" "[0-9]{3}" ".+"\n" { parse_status_line(&yyextra->response_status_line, yytext); }
^("GET"|"PUT"|"POST")" "([^ ]+)" HTTP/"([^ ]+)"\n" {
  parse_method(&yyextra->request_method_info, yytext); }
^[^:\n]+": ".+"\n"+ { parse_header(yyextra->headers, yytext); }
.+ {
  puts(yytext);
  /* TODO: parse_body(); */ }
%%

/* Parse HTTP message */
  int parse_message(int client, char *message_buffer, struct message_t *http_request) {

  char *cl_conversion;
  char log_message[LOG_SIZE];

  /* Configure lex setup */
  yyscan_t scanner;
  YY_BUFFER_STATE buf;
  yylex_init(&scanner);
  yylex_init_extra((YY_EXTRA_TYPE)http_request, &scanner);
  buf = yy_scan_string(message_buffer, scanner);

  /* Actually parse the HTTP message */
  yylex(scanner);

  /* We are done with the scanner and buf */
  yy_delete_buffer(buf, scanner);
  yylex_destroy(scanner);

  /* Iterate through headers, check for Content-Length */
  for(int i = 0; i < HEADER_ARRAY_LENGTH; i++) {

    if(http_request->headers[i].header_name != NULL && http_request->headers[i].header_value != NULL) {

      /* We do not support Transfer-Encoding */
      if (strcmp(http_request->headers[i].header_name, "Transfer-Encoding") == 0) {
        strncpy(log_message, "Failure: Transfer-Encoding Header.", LOG_SIZE);
        // respond("501 (Not Implemented)");
        log_event(log_message);
        pthread_exit(NULL);
      }

      /* Content-Length's presence indicates an HTTP response or certain requests. */
      if (strcmp(http_request->headers[i].header_name, "Content-Length") == 0) {

        /* If the message is indeed a request, only POST requests will have the method line parsed. */
        /* If both these fields are NULL, it means a non-supported request method was sent. */
        if(http_request->request_method_info.method_type == NULL && http_request->response_status_line.http_protocol == NULL) {
            strncpy(log_message, "Failure: Unsupported Method.", LOG_SIZE);
            // respond("501 (Not Implemented)");
            log_event(log_message);
            pthread_exit(NULL);
        }

        http_request->data_type.is_response = 0;

        /* Reset errno to 0 before strtol call */
        errno = 0;

        /* Save content_length. */
        http_request->data_type.content_length = strtol(http_request->headers[i].header_value, &cl_conversion, 10);

        /* If errno is set, the 0 returned is invalid. */
        if (http_request->data_type.content_length == 0 && errno != 0){
          strncpy(log_message, "Failure: Could not convert Content-Length", LOG_SIZE);
          log_event(log_message);
          pthread_exit(NULL);
        }
      }
     }
   }

   /* If the message is a request (not containing Content-Length header), only supported methods are parsed. */
   if(http_request->data_type.is_response == 1) {
     if(http_request->request_method_info.method_type == NULL){
       strncpy(log_message, "Failure: Unsupported Method.", LOG_SIZE);
       // respond("501 (Not Implemented)");
       log_event(log_message);
       pthread_exit(NULL);
     }
   }

  /* Testing: Print method/status_line, and all parsed headers */

  //NOTE: Response fields only.
  // printf("Protocol:%s\n", http_request->response_status_line.http_protocol );
  // printf("Status-Code:%s\n", http_request->response_status_line.status_code );
  // printf("Reason-Phrase:%s\n", http_request->response_status_line.reason_phrase );

  //NOTE: Request fields only.
  // printf("method_line:%s\n", http_request->request_method_info.method_type );
  // printf("destination_uri:%s\n", http_request->request_method_info.destination_uri );
  // printf("http_protocol:%s\n", http_request->request_method_info.http_protocol );

  // for(int i = 0; i < HEADER_ARRAY_LENGTH; i++) {
  //   if(http_request->headers[i].header_name != NULL && http_request->headers[i].header_value != NULL) {
  //     printf("Name:%s\n", http_request->headers[i].header_name);
  //     printf("Value:%s", http_request->headers[i].header_value);
  //     printf("/* ----------------------*/ \n");
  //    }
  //  }

  return EXIT_SUCCESS;
}
