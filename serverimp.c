#include "serverdef.h"
#include "service.c"

void new_socket() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
}

void socket_init(char * arg) {
    bzero((char *) &serv_addr, sizeof(serv_addr));
    int portno = atoi(arg);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // Binds to all available interfaces
    serv_addr.sin_port = htons(portno);
}

void bind_host() {
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }
}

void start_listen(char *arg) {
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    printf("Server is listening on port %s...\n", arg);
}

void accept_connection() {
    new_sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (new_sockfd < 0) {
        perror("ERROR on accept");
        exit(1);
    }
}

int listen_question(int sock) {
    bzero(buffer, MAX_BUFFER);

    n = read(sock, buffer, MAX_BUFFER-1);
    if (n < 0) {
        perror("ERROR reading from socket");
        return -1 ;
    }
    if (n == 0) {
        printf("Client closed connection\n");
        return 5; // Return 5 to exit cleanly
    }

    return atoi(buffer) ;
}

int answer_question(int sock, int answer, time_t start_time){
     switch(answer){
        case 1 :
            
            date_time(buffer, MAX_BUFFER) ;

            n = write(sock, buffer, strlen(buffer)) ;
            if (n < 0){
                perror("ERROR writing to socket");
                return 0 ;
            }
            if (n < strlen(buffer)){
                perror("Partial write occurred");
                return 0 ;
            }
            
            return 1 ;
        case 2 :

            directory_files(buffer, MAX_BUFFER) ;
            n = write(sock, buffer, strlen(buffer)) ;
            if (n < 0){
                perror("ERROR writing to socket");
                return 0 ;
            }
            if (n < strlen(buffer)){
                perror("Partial write occurred");
                return 0 ;
            }
        
            return 1 ;
        case 3 :

            file_content(buffer, MAX_BUFFER, sock) ;
            
            // Continue the session after file transfer
            return 1 ;
        case 4 :
            session_time(buffer, MAX_BUFFER, start_time);
            n = write(sock, buffer, strlen(buffer));
            if (n < 0) {
                perror("ERROR writing to socket");
                return 0;
            }
            if (n < strlen(buffer)){
                perror("Partial write occurred");
                return 0 ;
            }
            return 1;
        case 5 :
            return 0 ;
        case -1 :
            // Error reading from socket, close connection
            return 0 ;
        default :
            // Invalid option
            strcpy(buffer, "Invalid option. Please try again.");
            n = write(sock, buffer, strlen(buffer));
            if (n < strlen(buffer)){
                return 0 ;
            }
            return 1 ;
    }
}

void handle_client(int sock) {
    time_t session_start_time;
    time(&session_start_time);

    int run = 1;
    char *menu = "\n========================================\n"
                 "         SERVER MENU\n"
                 "========================================\n"
                 "1. Show date and time\n"
                 "2. List directory files\n"
                 "3. Display file content\n"
                 "4. Show session elapsed time\n"
                 "5. Exit\n"
                 "========================================\n"
                 "Enter your choice: " ;

    while (run)
    {
        // Use the passed socket descriptor 'sock' for communication
        n = write(sock, menu, strlen(menu)) ;
        if (n < 0){
             perror("ERROR writing to socket");
             run = 0;
             continue ;
        }
        if (n < strlen(menu)){
             perror("Partial write of menu");
             run = 0;
             continue ;
        }

        // Pass the client-specific socket to the communication functions
        int answer = listen_question(sock) ;

        run = answer_question(sock, answer, session_start_time) ;
    }
    printf("Client disconnected. Closing socket %d.\n", sock);
    close(sock);
}