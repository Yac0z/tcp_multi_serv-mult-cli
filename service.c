#include "service.h"

void date_time(char *buffer, int max_buffer) {
    bzero(buffer, max_buffer);
    time_t raw_time ;
    struct tm *local_time_info ;

    time(&raw_time) ;

    local_time_info = localtime(&raw_time) ;

    strftime(buffer, max_buffer-1, "%Y-%m-%d %H:%M:%S", local_time_info) ;
}

void directory_files(char *buffer, int max_buffer) {
    bzero(buffer, max_buffer);

    DIR *d ;
    struct dirent *dir ;

    d = opendir("./data") ;

    if(d == NULL)
        strcpy(buffer, "nothing to show") ;
    else{
        while((dir = readdir(d)) != NULL && strlen(buffer)+strlen(dir->d_name) < max_buffer){
            strcat(buffer, dir->d_name) ;
            strcat(buffer, "\n") ;
        }
    }
    
    closedir(d) ;
}

void file_content(char *buffer, int max_buffer, int sockfd) {
    FILE *file_ptr ;
    bzero(buffer, max_buffer) ;
    size_t buffer_read ;
    size_t buffer_sent ;

    file_ptr = fopen("./data/my_data.txt", "r") ;
    if(file_ptr == NULL){
        // Using 'sockfd' which is passed as an argument. 'new_sockfd' was not defined.
        write(sockfd, "this file not exist", 20) ;
        return ;
    }else{
        // Loop based on fread's return value for correctness.
        while ((buffer_read = fread(buffer, 1, max_buffer, file_ptr)) > 0) {
            // Safely print the buffer content to server's stdout for debugging, adding a newline for clarity.
            printf("Sending chunk: %.*s\n", (int)buffer_read, buffer);
            fflush(stdout);

            buffer_sent = send(sockfd, buffer, buffer_read, 0);
            if (buffer_sent < 0) {
                perror("send failed");
                break; // Exit loop on send error
            }
            bzero(buffer, max_buffer);
        }
        fclose(file_ptr) ;
    }
}

void session_time(char *buffer, int max_buffer, time_t start_time) {
    bzero(buffer, max_buffer);
    time_t current_time;
    double elapsed_seconds;

    time(&current_time);
    elapsed_seconds = difftime(current_time, start_time);

    snprintf(buffer, max_buffer - 1, "Current session has been active for %.0f seconds.", elapsed_seconds);
}