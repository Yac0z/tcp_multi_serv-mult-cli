#include "clientdef.h"

int main(int argc, char *argv[])
{
    int run = 1;
    
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);

    // 1. Create a socket
    new_socket(argv[1]) ;

    socket_init(portno) ;

    // 2. Connect to the server
    connect_server() ;

    // 3. Communicate with normal client mode
    run_normal_client();

    close_connection() ;
    return 0;
}
