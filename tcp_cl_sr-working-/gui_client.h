#ifndef GUI_CLIENT_H
#define GUI_CLIENT_H

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_BUFFER 256

// Global GTK widgets
typedef struct {
    GtkWidget *window;
    GtkWidget *stack;
    
    // Connection page widgets
    GtkWidget *host_entry;
    GtkWidget *port_entry;
    GtkWidget *connect_button;
    GtkWidget *status_label;
    
    // Auth page widgets
    GtkWidget *auth_stack;
    GtkWidget *auth_username_entry;
    GtkWidget *auth_password_entry;
    GtkWidget *auth_status_label;
    GtkWidget *login_button;
    GtkWidget *register_button;
    GtkWidget *auth_toggle_button;
    
    // Main menu widgets
    GtkWidget *datetime_button;
    GtkWidget *listfiles_button;
    GtkWidget *readfile_button;
    GtkWidget *sessiontime_button;
    GtkWidget *disconnect_button;
    GtkWidget *result_textview;
    GtkTextBuffer *result_buffer;
    
    // File path dialog
    GtkWidget *filepath_entry;
    
    // Socket information
    int sockfd;
    int connected;
    char hostname[256];
    int port;
} AppWidgets;

// Function declarations
void create_connection_page(AppWidgets *widgets);
void create_auth_page(AppWidgets *widgets);
void create_main_menu_page(AppWidgets *widgets);
void on_connect_clicked(GtkWidget *widget, gpointer data);
void on_login_clicked(GtkWidget *widget, gpointer data);
void on_register_clicked(GtkWidget *widget, gpointer data);
void on_datetime_clicked(GtkWidget *widget, gpointer data);
void on_listfiles_clicked(GtkWidget *widget, gpointer data);
void on_readfile_clicked(GtkWidget *widget, gpointer data);
void on_sessiontime_clicked(GtkWidget *widget, gpointer data);
void on_disconnect_clicked(GtkWidget *widget, gpointer data);
void on_window_destroy(GtkWidget *widget, gpointer data);

// Network functions
int connect_to_server(const char *hostname, int port);
int authenticate_user(int sockfd, const char *username, const char *password, int is_register);
int send_request(int sockfd, const char *request);
int receive_response(int sockfd, char *response, size_t max_size);
void disconnect_from_server(int sockfd);

#endif // GUI_CLIENT_H
