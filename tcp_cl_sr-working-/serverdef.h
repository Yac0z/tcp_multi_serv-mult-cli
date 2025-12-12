#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_BUFFER 256


int sockfd, new_sockfd;

socklen_t clilen;

char buffer[MAX_BUFFER];

struct sockaddr_in serv_addr, cli_addr;

int n ;

// 1. Create a socket
void new_socket() ;

// Initialize socket structure
void socket_init(char *arg) ;

// 2. Bind the host address
void bind_host() ;

// 3. Start listening for the clients
void start_listen(char* arg) ;

// 4. Accept actual connection from the client
void accept_connection() ;

// 5. Communicate
int listen_question(int sock) ;

int answer_question(int sock, int answer, time_t start_time) ;

//6. close communication
void close_server() ;

void handle_client(int sock);
