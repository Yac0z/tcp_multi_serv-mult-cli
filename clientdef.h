#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define MAX_BUFFER 256


int sockfd, portno, n;

struct sockaddr_in serv_addr;

struct hostent *server;

char buffer[MAX_BUFFER];

void new_socket(char *arg) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(arg);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
}

void socket_init(int portn) {
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
}

void connect_server() {
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }
}

char send_question(){
    bzero(buffer,MAX_BUFFER);
    fgets(buffer,MAX_BUFFER-1,stdin);

    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0){
        perror("ERROR reading from socket");
    }
    return buffer[0] ;
}

int show_answer(){
    char response[4096];  // Larger buffer for file content
    bzero(response, sizeof(response)); 
    n = read(sockfd, response, sizeof(response)-1);
    if (n < 0){
        perror("ERROR reading from socket");
        return 0 ;
    }
    if (n == 0){
        printf("Server closed connection\n");
        return 0 ;
    }
    printf("%s\n", response) ;
    return 1 ;
}

int reseve_answer(char answer){
    switch (answer)
    {
        case '1' :
            show_answer() ;
            return 1 ;
        case '2':
            show_answer() ;
            return 1 ;
        case '3' :
            show_answer() ;
            return 1 ;
        case '4' :
            show_answer();
            return 1;
        case '5' :
            return 0 ;
        default:
            return 1 ;
    }

}

void close_connection(){
    close(sockfd);
}