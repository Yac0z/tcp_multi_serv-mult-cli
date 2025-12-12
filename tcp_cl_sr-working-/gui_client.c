#include "gui_client.h"

// Global app widgets pointer
static AppWidgets *app_widgets = NULL;

// ============================================================================
// Network Functions
// ============================================================================

int connect_to_server(const char *hostname, int port) {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return -1;
    }
    
    // Get server by hostname
    server = gethostbyname(hostname);
    if (server == NULL) {
        close(sockfd);
        return -1;
    }
    
    // Initialize socket address structure
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(port);
    
    // Connect to server
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd);
        return -1;
    }
    
    return sockfd;
}

int authenticate_user(int sockfd, const char *username, const char *password, int is_register) {
    char auth_message[MAX_BUFFER];
    char auth_response[MAX_BUFFER];
    int n;
    
    // Build authentication message
    if (is_register) {
        snprintf(auth_message, MAX_BUFFER, "REGISTER:%s:%s", username, password);
    } else {
        snprintf(auth_message, MAX_BUFFER, "AUTH:%s:%s", username, password);
    }
    
    // Send authentication request
    n = write(sockfd, auth_message, strlen(auth_message));
    if (n < 0) {
        return -1;
    }
    
    // Receive authentication result
    bzero(auth_response, MAX_BUFFER);
    n = read(sockfd, auth_response, MAX_BUFFER - 1);
    if (n <= 0) {
        return -1;
    }
    auth_response[n] = '\0';
    
    // Check if authentication succeeded
    if (strncmp(auth_response, "AUTH_OK", 7) == 0) {
        return 0; // Success
    } else {
        return -2; // Failed authentication
    }
}

int send_request(int sockfd, const char *request) {
    int n = write(sockfd, request, strlen(request));
    return (n < 0) ? -1 : 0;
}

int receive_response(int sockfd, char *response, size_t max_size) {
    int total_read = 0;
    int n;
    fd_set readfds;
    struct timeval tv;
    
    bzero(response, max_size);
    
    // Read response (may come in multiple chunks for large files)
    while (total_read < max_size - 1) {
        // Set up select with timeout
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        tv.tv_sec = 2;  // 2 second timeout
        tv.tv_usec = 0;
        
        int ready = select(sockfd + 1, &readfds, NULL, NULL, &tv);
        
        if (ready < 0) {
            return -1;  // Error
        } else if (ready == 0) {
            // Timeout - no more data
            break;
        }
        
        // Data is available, read it
        n = read(sockfd, response + total_read, max_size - total_read - 1);
        
        if (n < 0) {
            return -1;  // Error
        } else if (n == 0) {
            // Connection closed
            break;
        }
        
        total_read += n;
    }
    
    response[total_read] = '\0';
    return total_read;
}

void disconnect_from_server(int sockfd) {
    if (sockfd >= 0) {
        close(sockfd);
    }
}

// ============================================================================
// GTK Callbacks - Connection Page
// ============================================================================

void on_connect_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    const char *hostname = gtk_entry_get_text(GTK_ENTRY(widgets->host_entry));
    const char *port_str = gtk_entry_get_text(GTK_ENTRY(widgets->port_entry));
    
    // Validate input
    if (strlen(hostname) == 0) {
        gtk_label_set_text(GTK_LABEL(widgets->status_label), "Error: Please enter hostname");
        return;
    }
    
    if (strlen(port_str) == 0) {
        gtk_label_set_text(GTK_LABEL(widgets->status_label), "Error: Please enter port");
        return;
    }
    
    int port = atoi(port_str);
    if (port <= 0 || port > 65535) {
        gtk_label_set_text(GTK_LABEL(widgets->status_label), "Error: Invalid port number");
        return;
    }
    
    // Attempt connection
    gtk_label_set_text(GTK_LABEL(widgets->status_label), "Connecting...");
    
    int sockfd = connect_to_server(hostname, port);
    if (sockfd < 0) {
        gtk_label_set_text(GTK_LABEL(widgets->status_label), "Error: Failed to connect to server");
        return;
    }
    
    // Save connection info
    widgets->sockfd = sockfd;
    widgets->connected = 1;
    strncpy(widgets->hostname, hostname, sizeof(widgets->hostname) - 1);
    widgets->port = port;
    
    gtk_label_set_text(GTK_LABEL(widgets->status_label), "Connected! Please authenticate.");
    
    // Switch to authentication page
    gtk_stack_set_visible_child_name(GTK_STACK(widgets->stack), "auth");
}

// ============================================================================
// GTK Callbacks - Authentication Page
// ============================================================================

void on_login_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    const char *username = gtk_entry_get_text(GTK_ENTRY(widgets->auth_username_entry));
    const char *password = gtk_entry_get_text(GTK_ENTRY(widgets->auth_password_entry));
    
    // Validate input
    if (strlen(username) == 0) {
        gtk_label_set_text(GTK_LABEL(widgets->auth_status_label), "Error: Please enter username");
        return;
    }
    
    if (strlen(password) == 0) {
        gtk_label_set_text(GTK_LABEL(widgets->auth_status_label), "Error: Please enter password");
        return;
    }
    
    // Attempt authentication
    gtk_label_set_text(GTK_LABEL(widgets->auth_status_label), "Authenticating...");
    
    int result = authenticate_user(widgets->sockfd, username, password, 0);
    
    if (result == 0) {
        gtk_label_set_text(GTK_LABEL(widgets->auth_status_label), "Login successful!");
        
        // Switch to main menu
        gtk_stack_set_visible_child_name(GTK_STACK(widgets->stack), "menu");
        
        // Clear password field
        gtk_entry_set_text(GTK_ENTRY(widgets->auth_password_entry), "");
    } else if (result == -2) {
        gtk_label_set_text(GTK_LABEL(widgets->auth_status_label), "Error: Invalid username or password");
    } else {
        gtk_label_set_text(GTK_LABEL(widgets->auth_status_label), "Error: Authentication failed");
    }
}

void on_register_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    const char *username = gtk_entry_get_text(GTK_ENTRY(widgets->auth_username_entry));
    const char *password = gtk_entry_get_text(GTK_ENTRY(widgets->auth_password_entry));
    
    // Validate input
    if (strlen(username) == 0) {
        gtk_label_set_text(GTK_LABEL(widgets->auth_status_label), "Error: Please enter username");
        return;
    }
    
    if (strlen(password) < 6) {
        gtk_label_set_text(GTK_LABEL(widgets->auth_status_label), "Error: Password must be at least 6 characters");
        return;
    }
    
    // Attempt registration
    gtk_label_set_text(GTK_LABEL(widgets->auth_status_label), "Registering...");
    
    int result = authenticate_user(widgets->sockfd, username, password, 1);
    
    if (result == 0) {
        gtk_label_set_text(GTK_LABEL(widgets->auth_status_label), "Registration successful!");
        
        // Switch to main menu
        gtk_stack_set_visible_child_name(GTK_STACK(widgets->stack), "menu");
        
        // Clear password field
        gtk_entry_set_text(GTK_ENTRY(widgets->auth_password_entry), "");
    } else if (result == -2) {
        gtk_label_set_text(GTK_LABEL(widgets->auth_status_label), "Error: Username already exists or invalid");
    } else {
        gtk_label_set_text(GTK_LABEL(widgets->auth_status_label), "Error: Registration failed");
    }
}

// ============================================================================
// GTK Callbacks - Main Menu Page
// ============================================================================

void on_datetime_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    char response[4096];
    
    // Send request
    if (send_request(widgets->sockfd, "1") < 0) {
        gtk_text_buffer_set_text(widgets->result_buffer, "Error: Failed to send request", -1);
        return;
    }
    
    // Receive response
    if (receive_response(widgets->sockfd, response, sizeof(response)) < 0) {
        gtk_text_buffer_set_text(widgets->result_buffer, "Error: Failed to receive response", -1);
        return;
    }
    
    // Display result
    gtk_text_buffer_set_text(widgets->result_buffer, response, -1);
}

void on_listfiles_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    char response[4096];
    
    // Send request
    if (send_request(widgets->sockfd, "2") < 0) {
        gtk_text_buffer_set_text(widgets->result_buffer, "Error: Failed to send request", -1);
        return;
    }
    
    // Receive response
    if (receive_response(widgets->sockfd, response, sizeof(response)) < 0) {
        gtk_text_buffer_set_text(widgets->result_buffer, "Error: Failed to receive response", -1);
        return;
    }
    
    // Display result
    gtk_text_buffer_set_text(widgets->result_buffer, response, -1);
}

void on_readfile_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    char response[8192];
    
    // Create dialog to ask for filename
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Enter Filename",
        GTK_WINDOW(widgets->window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_OK", GTK_RESPONSE_OK,
        NULL
    );
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "e.g., my_data.txt");
    gtk_container_add(GTK_CONTAINER(content_area), gtk_label_new("Filename in data directory:"));
    gtk_container_add(GTK_CONTAINER(content_area), entry);
    gtk_widget_show_all(dialog);
    
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_OK) {
        const char *filename = gtk_entry_get_text(GTK_ENTRY(entry));
        
        if (strlen(filename) == 0) {
            gtk_text_buffer_set_text(widgets->result_buffer, "Error: Please enter a filename", -1);
            gtk_widget_destroy(dialog);
            return;
        }
        
        // Send request option 3
        if (send_request(widgets->sockfd, "3") < 0) {
            gtk_text_buffer_set_text(widgets->result_buffer, "Error: Failed to send request", -1);
            gtk_widget_destroy(dialog);
            return;
        }
        
        // Small delay to ensure server is ready for filename
        usleep(10000);  // 10ms delay
        
        // Send filename (without newline, just the filename)
        if (write(widgets->sockfd, filename, strlen(filename)) < 0) {
            gtk_text_buffer_set_text(widgets->result_buffer, "Error: Failed to send filename", -1);
            gtk_widget_destroy(dialog);
            return;
        }
        
        // Receive response (file content)
        int bytes_received = receive_response(widgets->sockfd, response, sizeof(response));
        if (bytes_received < 0) {
            gtk_text_buffer_set_text(widgets->result_buffer, "Error: Failed to receive response", -1);
            gtk_widget_destroy(dialog);
            return;
        } else if (bytes_received == 0) {
            gtk_text_buffer_set_text(widgets->result_buffer, "Error: No response from server", -1);
            gtk_widget_destroy(dialog);
            return;
        }
        
        // Display result
        gtk_text_buffer_set_text(widgets->result_buffer, response, -1);
    }
    
    gtk_widget_destroy(dialog);
}

void on_sessiontime_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    char response[4096];
    
    // Send request
    if (send_request(widgets->sockfd, "4") < 0) {
        gtk_text_buffer_set_text(widgets->result_buffer, "Error: Failed to send request", -1);
        return;
    }
    
    // Receive response
    if (receive_response(widgets->sockfd, response, sizeof(response)) < 0) {
        gtk_text_buffer_set_text(widgets->result_buffer, "Error: Failed to receive response", -1);
        return;
    }
    
    // Display result
    gtk_text_buffer_set_text(widgets->result_buffer, response, -1);
}

void on_disconnect_clicked(GtkWidget *widget, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    
    // Send exit request
    send_request(widgets->sockfd, "5");
    
    // Disconnect
    disconnect_from_server(widgets->sockfd);
    widgets->connected = 0;
    widgets->sockfd = -1;
    
    // Clear result buffer
    gtk_text_buffer_set_text(widgets->result_buffer, "Disconnected from server.", -1);
    
    // Go back to connection page
    gtk_stack_set_visible_child_name(GTK_STACK(widgets->stack), "connection");
    
    // Reset status
    gtk_label_set_text(GTK_LABEL(widgets->status_label), "Ready to connect");
    gtk_label_set_text(GTK_LABEL(widgets->auth_status_label), "");
}

void on_window_destroy(GtkWidget *widget, gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    
    // Cleanup
    if (widgets->connected) {
        send_request(widgets->sockfd, "5");
        disconnect_from_server(widgets->sockfd);
    }
    
    gtk_main_quit();
}

// ============================================================================
// Page Creation Functions
// ============================================================================

void create_connection_page(AppWidgets *widgets) {
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(page, 20);
    gtk_widget_set_margin_end(page, 20);
    gtk_widget_set_margin_top(page, 20);
    gtk_widget_set_margin_bottom(page, 20);
    
    // Title
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='large' weight='bold'>TCP Client Connection</span>");
    gtk_box_pack_start(GTK_BOX(page), title, FALSE, FALSE, 10);
    
    // Host entry
    GtkWidget *host_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(host_box), gtk_label_new("Hostname:"), FALSE, FALSE, 0);
    widgets->host_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widgets->host_entry), "localhost");
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->host_entry), "localhost or IP address");
    gtk_box_pack_start(GTK_BOX(host_box), widgets->host_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(page), host_box, FALSE, FALSE, 0);
    
    // Port entry
    GtkWidget *port_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(port_box), gtk_label_new("Port:"), FALSE, FALSE, 0);
    widgets->port_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(widgets->port_entry), "8080");
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->port_entry), "8080");
    gtk_box_pack_start(GTK_BOX(port_box), widgets->port_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(page), port_box, FALSE, FALSE, 0);
    
    // Connect button
    widgets->connect_button = gtk_button_new_with_label("Connect");
    g_signal_connect(widgets->connect_button, "clicked", G_CALLBACK(on_connect_clicked), widgets);
    gtk_box_pack_start(GTK_BOX(page), widgets->connect_button, FALSE, FALSE, 10);
    
    // Status label
    widgets->status_label = gtk_label_new("Ready to connect");
    gtk_box_pack_start(GTK_BOX(page), widgets->status_label, FALSE, FALSE, 0);
    
    gtk_stack_add_named(GTK_STACK(widgets->stack), page, "connection");
}

void create_auth_page(AppWidgets *widgets) {
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(page, 20);
    gtk_widget_set_margin_end(page, 20);
    gtk_widget_set_margin_top(page, 20);
    gtk_widget_set_margin_bottom(page, 20);
    
    // Title
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='large' weight='bold'>Authentication</span>");
    gtk_box_pack_start(GTK_BOX(page), title, FALSE, FALSE, 10);
    
    // Username entry
    GtkWidget *user_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(user_box), gtk_label_new("Username:"), FALSE, FALSE, 0);
    widgets->auth_username_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->auth_username_entry), "Enter username");
    gtk_box_pack_start(GTK_BOX(user_box), widgets->auth_username_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(page), user_box, FALSE, FALSE, 0);
    
    // Password entry
    GtkWidget *pass_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(pass_box), gtk_label_new("Password:"), FALSE, FALSE, 0);
    widgets->auth_password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(widgets->auth_password_entry), FALSE);
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->auth_password_entry), "Enter password (min 6 chars)");
    gtk_box_pack_start(GTK_BOX(pass_box), widgets->auth_password_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(page), pass_box, FALSE, FALSE, 0);
    
    // Button box
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    
    widgets->login_button = gtk_button_new_with_label("Login");
    g_signal_connect(widgets->login_button, "clicked", G_CALLBACK(on_login_clicked), widgets);
    gtk_box_pack_start(GTK_BOX(button_box), widgets->login_button, TRUE, TRUE, 0);
    
    widgets->register_button = gtk_button_new_with_label("Register");
    g_signal_connect(widgets->register_button, "clicked", G_CALLBACK(on_register_clicked), widgets);
    gtk_box_pack_start(GTK_BOX(button_box), widgets->register_button, TRUE, TRUE, 0);
    
    gtk_box_pack_start(GTK_BOX(page), button_box, FALSE, FALSE, 10);
    
    // Status label
    widgets->auth_status_label = gtk_label_new("");
    gtk_box_pack_start(GTK_BOX(page), widgets->auth_status_label, FALSE, FALSE, 0);
    
    gtk_stack_add_named(GTK_STACK(widgets->stack), page, "auth");
}

void create_main_menu_page(AppWidgets *widgets) {
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(page, 20);
    gtk_widget_set_margin_end(page, 20);
    gtk_widget_set_margin_top(page, 20);
    gtk_widget_set_margin_bottom(page, 20);
    
    // Title
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='large' weight='bold'>Server Menu</span>");
    gtk_box_pack_start(GTK_BOX(page), title, FALSE, FALSE, 10);
    
    // Buttons
    widgets->datetime_button = gtk_button_new_with_label("Show Date and Time");
    g_signal_connect(widgets->datetime_button, "clicked", G_CALLBACK(on_datetime_clicked), widgets);
    gtk_box_pack_start(GTK_BOX(page), widgets->datetime_button, FALSE, FALSE, 5);
    
    widgets->listfiles_button = gtk_button_new_with_label("List Directory Files");
    g_signal_connect(widgets->listfiles_button, "clicked", G_CALLBACK(on_listfiles_clicked), widgets);
    gtk_box_pack_start(GTK_BOX(page), widgets->listfiles_button, FALSE, FALSE, 5);
    
    widgets->readfile_button = gtk_button_new_with_label("Display File Content");
    g_signal_connect(widgets->readfile_button, "clicked", G_CALLBACK(on_readfile_clicked), widgets);
    gtk_box_pack_start(GTK_BOX(page), widgets->readfile_button, FALSE, FALSE, 5);
    
    widgets->sessiontime_button = gtk_button_new_with_label("Show Session Time");
    g_signal_connect(widgets->sessiontime_button, "clicked", G_CALLBACK(on_sessiontime_clicked), widgets);
    gtk_box_pack_start(GTK_BOX(page), widgets->sessiontime_button, FALSE, FALSE, 5);
    
    widgets->disconnect_button = gtk_button_new_with_label("Disconnect");
    g_signal_connect(widgets->disconnect_button, "clicked", G_CALLBACK(on_disconnect_clicked), widgets);
    gtk_box_pack_start(GTK_BOX(page), widgets->disconnect_button, FALSE, FALSE, 5);
    
    // Result text view
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scroll, -1, 200);
    
    widgets->result_textview = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(widgets->result_textview), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(widgets->result_textview), GTK_WRAP_WORD);
    widgets->result_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->result_textview));
    
    gtk_container_add(GTK_CONTAINER(scroll), widgets->result_textview);
    gtk_box_pack_start(GTK_BOX(page), scroll, TRUE, TRUE, 5);
    
    gtk_stack_add_named(GTK_STACK(widgets->stack), page, "menu");
}

// ============================================================================
// Main Function
// ============================================================================

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    // Allocate widgets structure
    app_widgets = g_malloc(sizeof(AppWidgets));
    app_widgets->sockfd = -1;
    app_widgets->connected = 0;
    
    // Create main window
    app_widgets->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app_widgets->window), "TCP Client GUI");
    gtk_window_set_default_size(GTK_WINDOW(app_widgets->window), 600, 500);
    gtk_container_set_border_width(GTK_CONTAINER(app_widgets->window), 10);
    g_signal_connect(app_widgets->window, "destroy", G_CALLBACK(on_window_destroy), app_widgets);
    
    // Create stack for pages
    app_widgets->stack = gtk_stack_new();
    gtk_container_add(GTK_CONTAINER(app_widgets->window), app_widgets->stack);
    
    // Create pages
    create_connection_page(app_widgets);
    create_auth_page(app_widgets);
    create_main_menu_page(app_widgets);
    
    // Show connection page first
    gtk_stack_set_visible_child_name(GTK_STACK(app_widgets->stack), "connection");
    
    // Show window
    gtk_widget_show_all(app_widgets->window);
    
    gtk_main();
    
    // Cleanup
    g_free(app_widgets);
    
    return 0;
}
