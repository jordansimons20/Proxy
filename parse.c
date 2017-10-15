/* Methods called in http.l to further parse HTTP */
#include "proxy.h"

//Functions
/* -------------------------------------------------------------------------------------------------------*/

void parse_method(){
  printf("Method \n");
}
/* -------------------------------------------------------------------------------------------------------*/

void parse_header(struct request_t *headers){
  printf("Header \n");
    printf("%s \n %s \n", headers[0].header_name, headers[0].header_value);
}
