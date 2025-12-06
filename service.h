#ifndef SERVICE_H
#define SERVICE_H

#include <time.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>

void date_time(char *buffer, int max_buffer);
void directory_files(char *buffer, int max_buffer);
void file_content(char *buffer, int max_buffer, int sockfd);
void session_time(char *buffer, int max_buffer, time_t start_time);

#endif // SERVICE_H