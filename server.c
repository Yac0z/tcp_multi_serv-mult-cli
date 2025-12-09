#include "serverimp.c"
#include <sys/wait.h>

void run_multiprocess_server() {
    pid_t pid;
    
    printf("\n[INFO] Starting MULTI-PROCESS server (concurrent clients)...\n");
    
    while (1) {
        // 4. Accept actual connection from the client
        accept_connection() ;

        pid = fork();
        if (pid < 0) {
            perror("ERROR on fork");
            exit(1);
        }

        if (pid == 0) {  // Child process
            close(sockfd); // Child doesn't need the listener
            handle_client(new_sockfd);
            exit(0);
        } else {  // Parent process
            close(new_sockfd); // Parent doesn't need this
            // Clean up zombie processes
            while(waitpid(-1, NULL, WNOHANG) > 0);
        }
    }
}

void run_fifo_server() {
    printf("\n[INFO] Starting FIFO server (sequential clients)...\n");
    
    while (1) {
        // Accept connection from client
        accept_connection();
        
        // Handle client completely before accepting next one
        printf("[INFO] Client connected. Handling request...\n");
        handle_client(new_sockfd);
        printf("[INFO] Client finished. Ready for next client.\n");
        
        // Close the client socket
        close(new_sockfd);
    }
}

int main(int argc, char *argv[])
{
    int choice;
    
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        fprintf(stderr,"Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    // Display server mode selection menu
    printf("\n========================================\n");
    printf("      TCP SERVER - MODE SELECTION\n");
    printf("========================================\n");
    printf("1. Multi-process (Concurrent clients)\n");
    printf("2. FIFO/Sequential (One client at a time)\n");
    printf("========================================\n");
    printf("Enter your choice: ");
    
    if (scanf("%d", &choice) != 1) {
        fprintf(stderr, "ERROR: Invalid input\n");
        exit(1);
    }
    
    // Clear input buffer
    while (getchar() != '\n');

    // 1. Create a socket
    new_socket() ;

    // Initialize socket structure
    socket_init(argv[1]) ;

    // 2. Bind the host address
    bind_host() ;

    // 3. Start listening for the clients
    start_listen(argv[1]) ;

    // Run server based on selected mode
    switch(choice) {
        case 1:
            run_multiprocess_server();
            break;
        case 2:
            run_fifo_server();
            break;
        default:
            fprintf(stderr, "ERROR: Invalid choice. Using Multi-process mode.\n");
            run_multiprocess_server();
            break;
    }
    
    close(sockfd);
    return 0;
}