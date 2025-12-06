#include "clientdef.h"

int main(int argc, char *argv[])
{
    int run = 1 ;
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

    // 3. Communicate
    while (run)
    {
        bzero(buffer,MAX_BUFFER);

        n = read(sockfd, buffer, MAX_BUFFER-1);
        if (n < 0){
            perror("ERROR reading from socket");
            run = 0 ;
            continue ;
        }

        printf("%s\n", buffer);

        char answer = send_question() ;

        run = reseve_answer(answer) ;

    }
    /*printf("Please enter the message: ");
    bzero(buffer,MAX_BUFFER);
    fgets(buffer,MAX_BUFFER-1,stdin);

    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) perror("ERROR writing to socket");

    bzero(buffer,MAX_BUFFER);
    n = read(sockfd, buffer, MAX_BUFFER-1);
    if (n < 0) perror("ERROR reading from socket");
    printf("%s\n", buffer);*/
    close_connection() ;
    return 0;
}