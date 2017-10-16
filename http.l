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
%option extra-type="struct request_t*"
%%
%{
  struct request_t *headers = yyextra;
%}

^("GET"|"PUT"|"POST")" "([^ ]+)" HTTP/"([^ ]+)"\n" {
  //foo[0] = 12345;
  parse_method(); }
^[^:\n]+": ".+"\n"+ { parse_header(yyextra, yytext); }
.+ {
  puts(yytext);
  /* TODO: parse_body(); */ }
%%

/* Parse HTTP request */
  int parse_request(int client, char *request_buffer, struct request_t *header_array) {

    // char log_message[LOG_SIZE];


  /* Configure lex setup */
  yyscan_t scanner;
  YY_BUFFER_STATE buf;
  // int thread_data[10]; //TODO: Replace with pointer to request_t struct
  yylex_init(&scanner);
  yylex_init_extra((YY_EXTRA_TYPE)header_array, &scanner);
  // yyset_in(stdin, scanner);
  buf = yy_scan_string(request_buffer, scanner);

  /* Actually parse the HTTP request */
  yylex(scanner);

  /* We are done with the scanner and buf */
  yy_delete_buffer(buf, scanner);
  yylex_destroy(scanner);

  //printf("%d\n", thread_data[0]);

  char request_buffer_final[REQUEST_SIZE + 100];

  /* Form response */
  strcpy(request_buffer_final,"HTTP/1.x 200 OK\nContent-Type: text/html\n\n" );
  strcat(request_buffer_final, request_buffer);
  respond(client, request_buffer_final);

  // free(request_buffer);
  authenticate();
  return EXIT_SUCCESS;
}
