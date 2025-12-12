#include "serverdef.h"
#include "auth.h"
#include "service.h"

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
            // Read filepath from client
            bzero(buffer, MAX_BUFFER);
            n = read(sock, buffer, MAX_BUFFER-1);
            if (n <= 0) {
                strcpy(buffer, "ERROR: No filepath provided");
                write(sock, buffer, strlen(buffer));
                return 1;
            }
            
            // Remove newline if present
            buffer[strcspn(buffer, "\n")] = 0;
            
            // Call file_content with the provided filepath
            file_content(buffer, MAX_BUFFER, sock, buffer);
            
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

    // Reload users in case they were updated by another process
    load_users();

    // Authentication phase
    printf("[AUTH] Client connected. Starting authentication...\n");
    
    // Receive authentication or registration request
    // Format: "AUTH:username:password" or "REGISTER:username:password"
    char auth_buffer[MAX_BUFFER];
    bzero(auth_buffer, MAX_BUFFER);
    n = read(sock, auth_buffer, MAX_BUFFER - 1);
    if (n <= 0) {
        printf("[AUTH] Failed to receive credentials\n");
        close(sock);
        return;
    }
    auth_buffer[n] = '\0';
    
    // Parse the request - use a copy to preserve original
    char auth_copy[MAX_BUFFER];
    strncpy(auth_copy, auth_buffer, MAX_BUFFER - 1);
    auth_copy[MAX_BUFFER - 1] = '\0';
    
    char *command = strtok(auth_copy, ":");
    char *username = strtok(NULL, ":");
    char *password = strtok(NULL, ":");
    
    if (!command || !username || !password) {
        printf("[AUTH] Invalid authentication format\n");
        char fail_msg[] = "AUTH_FAILED:Invalid format";
        write(sock, fail_msg, strlen(fail_msg));
        close(sock);
        return;
    }
    
    // Make copies of username and password to ensure they're not modified
    char stored_username[MAX_USERNAME];
    char stored_password[MAX_PASSWORD];
    
    strncpy(stored_username, username, MAX_USERNAME - 1);
    stored_username[MAX_USERNAME - 1] = '\0';
    
    strncpy(stored_password, password, MAX_PASSWORD - 1);
    stored_password[MAX_PASSWORD - 1] = '\0';
    
    printf("[AUTH] Received - Command: %s, Username: %s, Password length: %zu\n", 
           command, stored_username, strlen(stored_password));
    
    int auth_success = 0;
    
    if (strcmp(command, "REGISTER") == 0) {
        // Handle registration
        int reg_result = register_user(stored_username, stored_password);
        if (reg_result == 0) {
            auth_success = 1;
            printf("[AUTH] User registered successfully: %s\n", stored_username);
        } else {
            char fail_msg[MAX_BUFFER];
            if (reg_result == -1) {
                snprintf(fail_msg, MAX_BUFFER, "AUTH_FAILED:Username already exists");
            } else if (reg_result == -2) {
                snprintf(fail_msg, MAX_BUFFER, "AUTH_FAILED:Too many users");
            } else if (reg_result == -3) {
                snprintf(fail_msg, MAX_BUFFER, "AUTH_FAILED:Invalid username or password format");
            } else {
                snprintf(fail_msg, MAX_BUFFER, "AUTH_FAILED:Registration failed");
            }
            printf("[AUTH] Registration failed for user: %s (code: %d)\n", stored_username, reg_result);
            write(sock, fail_msg, strlen(fail_msg));
            close(sock);
            return;
        }
    } else if (strcmp(command, "AUTH") == 0) {
        // Handle login
        if (verify_credentials(stored_username, stored_password)) {
            auth_success = 1;
            printf("[AUTH] User authenticated: %s\n", stored_username);
        } else {
            printf("[AUTH] Authentication failed for user: %s\n", stored_username);
            char fail_msg[] = "AUTH_FAILED:Invalid credentials";
            write(sock, fail_msg, strlen(fail_msg));
            close(sock);
            return;
        }
    } else {
        printf("[AUTH] Unknown command: %s\n", command);
        char fail_msg[] = "AUTH_FAILED:Unknown command";
        write(sock, fail_msg, strlen(fail_msg));
        close(sock);
        return;
    }
    
    // Create session
    if (auth_success) {
        char token[TOKEN_SIZE * 2 + 1];
        if (!create_session(stored_username, token)) {
            printf("[AUTH] Failed to create session for user: %s\n", stored_username);
            char fail_msg[] = "AUTH_FAILED:Session creation failed";
            write(sock, fail_msg, strlen(fail_msg));
            close(sock);
            return;
        }
        
        // Send success message
        char success_msg[MAX_BUFFER];
        snprintf(success_msg, MAX_BUFFER, "AUTH_OK:%s", token);
        write(sock, success_msg, strlen(success_msg));
        printf("[AUTH] Session created for user: %s\n", stored_username);
    }

    // Clear buffer for normal operation
    bzero(buffer, MAX_BUFFER);
    
    int run = 1;
    printf("[INFO] Starting main communication loop for user: %s\n", stored_username);

    while (run)
    {
        // Use the passed socket descriptor 'sock' for communication

        // Pass the client-specific socket to the communication functions
        int answer = listen_question(sock) ;

        run = answer_question(sock, answer, session_start_time) ;
    }
    printf("[INFO] Client disconnected: %s (socket %d)\n", stored_username, sock);
    close(sock);
}
