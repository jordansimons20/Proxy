/* Control functionality for local requests */
#include "proxy.h"

//Function Prototypes

//Functions
/* -------------------------------------------------------------------------------------------------------*/
/* Close the socket */
int stop_server(int server) {
  return close(server);
}
/* -------------------------------------------------------------------------------------------------------*/
