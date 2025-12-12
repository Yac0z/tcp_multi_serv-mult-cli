#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <termios.h>

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

// Function to disable echo for password input
void disable_echo() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

// Function to enable echo
void enable_echo() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

// Function to handle authentication
int authenticate() {
    char auth_response[MAX_BUFFER];
    char username[64];
    char password[128];
    char choice[10];
    char auth_message[MAX_BUFFER];
    
    printf("\n========================================\n");
    printf("         AUTHENTICATION\n");
    printf("========================================\n");
    printf("1. Register new account\n");
    printf("2. Login with existing account\n");
    printf("========================================\n");
    printf("Enter your choice (1-2): ");
    
    if (fgets(choice, sizeof(choice), stdin) == NULL) {
        fprintf(stderr, "ERROR: Failed to read choice\n");
        return 0;
    }
    choice[strcspn(choice, "\n")] = 0;
    
    int is_register = (strcmp(choice, "1") == 0);
    
    // Get username
    printf("Username: ");
    if (fgets(username, sizeof(username), stdin) == NULL) {
        fprintf(stderr, "ERROR: Failed to read username\n");
        return 0;
    }
    username[strcspn(username, "\n")] = 0; // Remove newline
    
    // Get password (without echo)
    printf("Password: ");
    disable_echo();
    if (fgets(password, sizeof(password), stdin) == NULL) {
        enable_echo();
        fprintf(stderr, "\nERROR: Failed to read password\n");
        return 0;
    }
    enable_echo();
    printf("\n");
    password[strcspn(password, "\n")] = 0; // Remove newline
    
    // Build authentication message
    if (is_register) {
        snprintf(auth_message, MAX_BUFFER, "REGISTER:%s:%s", username, password);
    } else {
        snprintf(auth_message, MAX_BUFFER, "AUTH:%s:%s", username, password);
    }
    
    // Send authentication/registration request
    n = write(sockfd, auth_message, strlen(auth_message));
    if (n < 0) {
        perror("ERROR: Failed to send credentials");
        return 0;
    }
    
    // Wait for authentication result
    bzero(auth_response, MAX_BUFFER);
    n = read(sockfd, auth_response, MAX_BUFFER - 1);
    if (n <= 0) {
        fprintf(stderr, "ERROR: Failed to receive authentication result\n");
        return 0;
    }
    auth_response[n] = '\0';
    
    if (strncmp(auth_response, "AUTH_OK", 7) == 0) {
        if (is_register) {
            printf("[AUTH] Registration successful! You are now logged in.\n");
        } else {
            printf("[AUTH] Authentication successful!\n");
        }
        
        // Extract token if provided
        char *token = strchr(auth_response, ':');
        if (token != NULL) {
            token++; // Skip the ':'
            printf("[AUTH] Session token received\n");
        }
        return 1;
    } else if (strncmp(auth_response, "AUTH_FAILED", 11) == 0) {
        // Extract error message if provided
        char *error_msg = strchr(auth_response, ':');
        if (error_msg != NULL) {
            error_msg++; // Skip the ':'
            fprintf(stderr, "[AUTH] Failed: %s\n", error_msg);
        } else {
            fprintf(stderr, "[AUTH] Authentication/Registration failed!\n");
        }
        return 0;
    } else {
        fprintf(stderr, "[AUTH] Unexpected server response: %s\n", auth_response);
        return 0;
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
    char response[1024];  // Larger buffer for file content
    bzero(response, sizeof(response)); 
    while(n = read(sockfd, response, sizeof(response)-1) == 1023){
        response[1023] = '\0' ;
    if (n < 0){
        perror("ERROR reading from socket");
        return 0 ;
    }
    if (n == 0){
        printf("Server closed connection\n");
        return 0 ;
    }
    printf("%s\n", response) ;
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
            // Option 3: Ask for file path
            bzero(buffer, MAX_BUFFER);
            printf("Enter file path in data directory: ");
            fgets(buffer, MAX_BUFFER-1, stdin);
            
            // Remove newline if present
            buffer[strcspn(buffer, "\n")] = 0;
            
            // Send filepath to server
            n = write(sockfd, buffer, strlen(buffer));
            if (n < 0) {
                perror("ERROR writing to socket");
            }
            
            // Receive and display the file content
            show_answer();
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

void run_normal_client() {
    int run = 1;
    
    // Authenticate first
    if (!authenticate()) {
        fprintf(stderr, "ERROR: Authentication failed. Disconnecting.\n");
        return;
    }
    
    printf("\n[INFO] Connected to server in NORMAL mode.\n");
    
    while (run)
    {
        bzero(buffer,MAX_BUFFER);

        printf("\n========================================\n") ;
        printf("         SERVER MENU\n") ;
        printf("========================================\n") ;
        printf("1. Show date and time\n") ;
        printf("2. List directory files\n") ;
        printf("3. Display file content (specify path)\n") ;
        printf("4. Show session elapsed time\n") ;
        printf("5. Exit\n") ;
        printf("========================================\n") ;
        printf("Enter your choice: ") ;

        char answer = send_question() ;

        run = reseve_answer(answer) ;
    }
}

void run_mono_client() {
    // Authenticate first
    if (!authenticate()) {
        fprintf(stderr, "ERROR: Authentication failed. Disconnecting.\n");
        return;
    }
    
    printf("\n[INFO] Connected to server in MONO mode.\n");
    printf("[INFO] Single session - make one request and disconnect.\n\n");
    
    bzero(buffer,MAX_BUFFER);

    printf("========================================\n") ;
    printf("         SERVER MENU (MONO)\n") ;
    printf("========================================\n") ;
    printf("1. Show date and time\n") ;
    printf("2. List directory files\n") ;
    printf("3. Display file content (specify path)\n") ;
    printf("4. Show session elapsed time\n") ;
    printf("5. Exit\n") ;
    printf("========================================\n") ;
    printf("Enter your choice: ") ;

    char answer = send_question() ;

    reseve_answer(answer) ;
}
