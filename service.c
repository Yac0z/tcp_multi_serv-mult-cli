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
    char file_buffer[4096];  // Larger buffer for file content
    bzero(file_buffer, sizeof(file_buffer));

    file_ptr = fopen("./data/my_data.txt", "r") ;
    if(file_ptr == NULL){
        strcpy(buffer, "ERROR: File does not exist");
        write(sockfd, buffer, strlen(buffer)) ;
        return ;
    }else{
        // Read entire file into buffer
        size_t bytes_read = fread(file_buffer, 1, sizeof(file_buffer) - 1, file_ptr);
        file_buffer[bytes_read] = '\0';  // Null terminate
        
        // Send the file content
        ssize_t buffer_sent = write(sockfd, file_buffer, bytes_read);
        if (buffer_sent < 0) {
            perror("send failed");
        } else if (buffer_sent < bytes_read) {
            perror("Partial write of file content");
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