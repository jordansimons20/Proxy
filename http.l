%{
#include <stdio.h>
#include "proxy.h"
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
^[^:\n\r]+": ".+[\n\r]+ { parse_header(yyextra->headers, yytext); }
.+ { puts("WARNING: THIS SHOULD NOT BE HERE!");
  for(int i = 0; i < strlen(yytext); i++) {
  printf("[%u]\n", (unsigned char) yytext[i]);
} }
%%

/* Parse HTTP message */
  int parse_message(char *message_buffer, struct message_t *http_request) {

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

  /* Iterate through headers, looking for "Content-Length" */
  check_content_length(http_request);

  /* Ensure HTTP Request uses supported methods. */
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
  printf("Protocol:%s\n", http_request->response_status_line.http_protocol );
  printf("Status-Code:%s\n", http_request->response_status_line.status_code );
  printf("Reason-Phrase:%s\n", http_request->response_status_line.reason_phrase );

  //NOTE: Request fields only.
  // printf("method_line:%s\n", http_request->request_method_info.method_type );
  // printf("destination_uri:%s\n", http_request->request_method_info.destination_uri.original_destination_uri );
  // printf("http_protocol:%s\n", http_request->request_method_info.http_protocol );
  // printf("Original Method_Line Length: %d\n", http_request->request_method_info.original_length);
  // printf("Host: %s\n", http_request->request_method_info.destination_uri.host);
  // printf("Path: %s\n", http_request->request_method_info.destination_uri.absolute_path);
  // printf("Port: %s\n", http_request->request_method_info.destination_uri.port);
    // printf("Relative method_line: %s\n",  http_request->request_method_info.relative_method_line);

  // for(int i = 0; i < HEADER_ARRAY_LENGTH; i++) {
  //   if(http_request->headers[i].header_name != NULL && http_request->headers[i].header_value != NULL) {
  //     printf("Name:%s\n", http_request->headers[i].header_name);
  //     printf("Value:%s", http_request->headers[i].header_value);
  //     printf("/* ----------------------*/ \n");
  //    }
  //  }

  return EXIT_SUCCESS;
}
