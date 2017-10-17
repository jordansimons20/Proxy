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
  struct request_t *http_request = yyextra;
%}

^("GET"|"PUT"|"POST")" "([^ ]+)" HTTP/"([^ ]+)"\n" {
  parse_method(yyextra->method, yytext); }
^[^:\n]+": ".+"\n"+ { parse_header(yyextra->headers, yytext); }
.+ {
  puts(yytext);
  /* TODO: parse_body(); */ }
%%

/* Parse HTTP request */
  int parse_request(int client, char *request_buffer, struct request_t *http_request) {

  /* Configure lex setup */
  yyscan_t scanner;
  YY_BUFFER_STATE buf;
  yylex_init(&scanner);
  yylex_init_extra((YY_EXTRA_TYPE)http_request, &scanner);
  buf = yy_scan_string(request_buffer, scanner);
  // yyset_in(stdin, scanner);

  /* Actually parse the HTTP request */
  yylex(scanner);

  /* We are done with the scanner and buf */
  yy_delete_buffer(buf, scanner);
  yylex_destroy(scanner);

  /* Testing: Print all parsed headers */
  for(int i = 0; i < HEADER_ARRAY_LENGTH; i++) {
    if(http_request->headers[i].header_name != NULL && http_request->headers[i].header_value != NULL) {
      printf("Name:%s\n", http_request->headers[i].header_name);
      printf("Value:%s", http_request->headers[i].header_value);
       printf("/* ----------------------*/ \n");
     }
   }

  char request_buffer_final[REQUEST_SIZE + 100];

  /* Form response */
  strcpy(request_buffer_final,"HTTP/1.x 200 OK\nContent-Type: text/html\n\n" );
  strcat(request_buffer_final, request_buffer);
  respond(client, request_buffer_final);

  authenticate();
  return EXIT_SUCCESS;
}
