#include "serverimp.c"
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    pid_t pid;
    
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    // 1. Create a socket
    new_socket() ;

    // Initialize socket structure
    socket_init(argv[1]) ;

    // 2. Bind the host address
    bind_host() ;

    // 3. Start listening for the clients
    start_listen(argv[1]) ;

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
    
    close(sockfd);
    return 0;
}