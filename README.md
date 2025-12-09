# TCP Multi-Server Multi-Client Application

## Project Overview

This is a comprehensive TCP client-server application that demonstrates two different server architectures:
1. **Multi-process Server** (Concurrent): Handles multiple clients simultaneously using process forking
2. **FIFO Server** (Sequential): Handles clients one at a time in a First-In-First-Out manner

The application provides various services including date/time display, directory listing, file content viewing, and session time tracking.

---

## Architecture

### Server Components

#### 1. **server.c** - Main Server Entry Point
```c
main(int argc, char *argv[])
```
- **Purpose**: Entry point that allows user to select server mode
- **Flow**:
  1. Validates command-line arguments (port number required)
  2. Displays server mode selection menu
  3. Creates and configures socket
  4. Binds to specified port
  5. Starts listening for connections
  6. Launches selected server mode (multi-process or FIFO)

**Key Functions**:
- `run_multiprocess_server()`: Implements concurrent client handling using fork()
- `run_fifo_server()`: Implements sequential client handling

#### 2. **serverimp.c** - Server Implementation
Contains all server logic and client handling functions.

**Socket Setup Functions**:
```c
void new_socket()
```
- Creates a TCP socket using `socket(AF_INET, SOCK_STREAM, 0)`
- AF_INET = IPv4, SOCK_STREAM = TCP protocol
- Stores socket file descriptor in global `sockfd`

```c
void socket_init(char *arg)
```
- Initializes the server address structure (`struct sockaddr_in`)
- Converts port number from string to integer using `atoi()`
- Sets address family to IPv4
- Binds to all available network interfaces (`INADDR_ANY`)
- Converts port to network byte order using `htons()`

```c
void bind_host()
```
- Binds the socket to the specified IP address and port
- Makes the socket available for accepting connections
- Exits on failure

```c
void start_listen(char *arg)
```
- Puts socket in listening mode with backlog of 5 connections
- Backlog = maximum number of pending connections in queue
- Initializes client address structure length

```c
void accept_connection()
```
- Blocks until a client connects
- Returns new socket descriptor (`new_sockfd`) for the connected client
- Original socket (`sockfd`) continues listening for new connections
- Exits on failure

**Client Communication Functions**:
```c
int listen_question(int sock)
```
- Reads client's menu choice from the socket
- Uses `read()` which blocks until data arrives
- Handles three cases:
  - `n < 0`: Read error - returns -1
  - `n == 0`: Client disconnected - returns 5 (exit code)
  - `n > 0`: Success - converts string to integer using `atoi()`

```c
int answer_question(int sock, int answer, time_t start_time)
```
- Processes client's choice and sends appropriate response
- **Case 1**: Date/Time
  - Calls `date_time()` to format current timestamp
  - Writes response to socket
  - Checks for partial writes
  - Returns 1 to continue session
- **Case 2**: Directory Listing
  - Calls `directory_files()` to list files in ./data/
  - Writes file list to socket
  - Returns 1 to continue session
- **Case 3**: File Content
  - Calls `file_content()` to read and send file
  - Uses 4096-byte buffer to handle large files
  - Returns 1 to continue session
- **Case 4**: Session Time
  - Calls `session_time()` to calculate elapsed time
  - Returns 1 to continue session
- **Case 5**: Exit
  - Returns 0 to terminate session
- **Case -1**: Error
  - Returns 0 to close connection
- **Default**: Invalid choice
  - Sends error message
  - Returns 1 to continue session

**Partial Write Handling**:
```c
if (n < strlen(buffer)){
    perror("Partial write occurred");
    return 0;
}
```
- TCP `write()` may send fewer bytes than requested if send buffer is full
- This check ensures all data was sent
- Prevents client-server synchronization issues

```c
void handle_client(int sock)
```
- Main client session handler
- **Flow**:
  1. Records session start time
  2. Initializes menu string with formatted options
  3. Enters main loop:
     - Sends menu to client
     - Checks for write errors and partial writes
     - Reads client's choice
     - Processes choice and sends response
     - Continues if client didn't choose exit
  4. Closes socket when client disconnects

#### 3. **serverdef.h** - Server Header
Defines server data structures and function prototypes.

**Global Variables**:
```c
int sockfd, new_sockfd;  // Socket descriptors
socklen_t clilen;         // Client address length
char buffer[MAX_BUFFER];  // Communication buffer (256 bytes)
struct sockaddr_in serv_addr, cli_addr;  // Address structures
int n;                    // I/O operation return value
```

**Why Global Variables?**
- Simplifies socket sharing between functions
- After `fork()`, child process gets copy of all globals
- Each process works with its own socket descriptors

#### 4. **service.c** - Service Implementation
Contains business logic for each service.

```c
void date_time(char *buffer, int max_buffer)
```
- Gets current system time using `time(&raw_time)`
- Converts to local time using `localtime()`
- Formats as "YYYY-MM-DD HH:MM:SS" using `strftime()`
- Stores result in buffer

```c
void directory_files(char *buffer, int max_buffer)
```
- Opens "./data" directory using `opendir()`
- Reads directory entries using `readdir()` in a loop
- Appends each filename to buffer with newline
- Handles case where directory doesn't exist
- Closes directory with `closedir()`

```c
void file_content(char *buffer, int max_buffer, int sockfd)
```
- Uses local 4096-byte buffer for large files
- Opens "./data/my_data.txt" using `fopen()`
- Reads entire file using `fread()`
- Sends content directly through socket using `write()`
- Handles file not found error
- Checks for partial writes
- Closes file with `fclose()`

**Why separate 4096-byte buffer?**
- Global buffer is only 256 bytes
- Files can be larger
- Direct write from local buffer avoids overwriting global buffer

```c
void session_time(char *buffer, int max_buffer, time_t start_time)
```
- Gets current time
- Calculates difference from session start using `difftime()`
- Formats message with elapsed seconds using `snprintf()`

---

### Client Components

#### 1. **client.c** - Client Entry Point
```c
main(int argc, char *argv[])
```
- **Purpose**: Connects to server and manages communication loop
- **Flow**:
  1. Validates arguments (hostname and port required)
  2. Creates socket
  3. Resolves hostname and initializes server address
  4. Connects to server
  5. Enters main communication loop:
     - Reads menu from server
     - Displays menu to user
     - Gets user input
     - Sends choice to server
     - Receives and displays response
     - Repeats until user chooses exit or error occurs
  6. Closes connection

**Connection Closure Handling**:
```c
if (n == 0){
    printf("Server closed connection\n");
    run = 0;
    continue;
}
```
- `read()` returns 0 when server closes connection
- Client detects this and exits gracefully

#### 2. **clientdef.h** - Client Implementation
Contains all client-side functions.

```c
void new_socket(char *arg)
```
- Creates TCP socket
- Resolves hostname using `gethostbyname()`
- Converts hostname to IP address
- Stores server information

```c
void socket_init(int portn)
```
- Initializes server address structure
- Copies server IP address from hostname resolution
- Sets port number in network byte order

```c
void connect_server()
```
- Establishes TCP connection to server using `connect()`
- Performs three-way handshake
- Blocks until connection established or fails

```c
char send_question()
```
- Clears buffer with `bzero()`
- Reads user input from stdin using `fgets()`
- Sends input to server using `write()`
- Returns first character (user's choice)

**Why return first character?**
- User types "1\n" but we only need '1'
- First character is the actual choice
- Used in switch statement in `reseve_answer()`

```c
int show_answer()
```
- Uses large 4096-byte local buffer for file content
- Reads server response using `read()`
- Handles three cases:
  - `n < 0`: Error
  - `n == 0`: Server closed connection
  - `n > 0`: Success - displays response
- Returns 1 on success, 0 on failure

```c
int reseve_answer(char answer)
```
- Routes to appropriate handler based on user's choice
- All options now use `show_answer()` for consistency
- Returns 1 to continue, 0 to exit
- **Case '1'**: Date/Time - display and continue
- **Case '2'**: Directory - display and continue
- **Case '3'**: File content - display and continue
- **Case '4'**: Session time - display and continue
- **Case '5'**: Exit - return 0
- **Default**: Continue (handles invalid input gracefully)

```c
void close_connection()
```
- Closes socket using `close(sockfd)`
- Releases system resources

---

## Server Architectures Explained

### 1. Multi-Process Server (Concurrent)

**How it works**:
```c
void run_multiprocess_server() {
    while (1) {
        accept_connection();  // Wait for client
        
        pid = fork();  // Create child process
        
        if (pid == 0) {  // Child process
            close(sockfd);           // Don't need listener
            handle_client(new_sockfd); // Handle client
            exit(0);                 // Exit when done
        } else {  // Parent process
            close(new_sockfd);       // Don't need client socket
            waitpid(-1, NULL, WNOHANG); // Clean up zombies
            // Loop back to accept next client
        }
    }
}
```

**Process Flow**:
1. Parent listens on port and accepts connection
2. `fork()` creates exact copy of parent process
3. Both processes have same variables and sockets
4. Child closes listening socket, keeps client socket
5. Parent closes client socket, keeps listening socket
6. Child handles client independently
7. Parent immediately accepts next client
8. Multiple children can run simultaneously

**Advantages**:
- Multiple clients can connect simultaneously
- Each client gets full CPU attention
- Client sessions are independent
- One client crash doesn't affect others

**Disadvantages**:
- Higher memory usage (one process per client)
- Context switching overhead
- Limited by system's process limit

**Zombie Process Cleanup**:
```c
while(waitpid(-1, NULL, WNOHANG) > 0);
```
- When child exits, becomes "zombie" until parent acknowledges
- `waitpid(-1, NULL, WNOHANG)` checks for finished children
- `-1` = any child process
- `WNOHANG` = non-blocking (returns immediately)
- Prevents accumulation of zombie processes

### 2. FIFO Server (Sequential)

**How it works**:
```c
void run_fifo_server() {
    while (1) {
        accept_connection();    // Wait for client
        handle_client(new_sockfd); // Handle completely
        close(new_sockfd);      // Close connection
        // Loop back to accept next client
    }
}
```

**Process Flow**:
1. Server accepts first client connection
2. Handles client's entire session (all requests)
3. Closes connection when client exits
4. Accepts next client connection
5. Other clients must wait in queue

**Advantages**:
- Simple design - no process management
- Low memory usage - only one client at a time
- Predictable behavior
- No race conditions

**Disadvantages**:
- Only one client served at a time
- Other clients wait in queue
- Long sessions block other clients
- Not suitable for high-traffic scenarios

**When to use FIFO**:
- Testing and development
- Low-traffic applications
- When client sessions are short
- When simplicity is priority

---

## Key Concepts Explained

### TCP Socket Communication

**Socket**: Endpoint for network communication (like a phone number)
**File Descriptor**: Integer that references the socket (like a handle)

**Server Socket Lifecycle**:
1. `socket()` - Create socket
2. `bind()` - Assign address and port
3. `listen()` - Mark as passive (accepting connections)
4. `accept()` - Wait for and accept client connection
5. `read()/write()` - Communicate with client
6. `close()` - Close connection

**Client Socket Lifecycle**:
1. `socket()` - Create socket
2. `connect()` - Connect to server
3. `read()/write()` - Communicate with server
4. `close()` - Close connection

### Blocking vs Non-Blocking I/O

**Blocking calls** (used in this project):
- `accept()` - Blocks until client connects
- `read()` - Blocks until data arrives
- `write()` - Blocks until data sent

**Why blocking is OK here**:
- Server: Each client is in separate process (multi-process) or handled sequentially (FIFO)
- Client: Interactive - waits for user input anyway

### Network Byte Order

```c
htons(portno)  // Host TO Network Short
```
- Different CPUs store multi-byte integers differently (endianness)
- Big-endian: Most significant byte first
- Little-endian: Least significant byte first
- Network protocols use big-endian
- `htons()` converts port number to network byte order

### Buffer Management

**Global buffer (256 bytes)**:
- Used for menu, date/time, directory listing
- Shared by all functions
- Cleared with `bzero()` before use

**Local buffers (4096 bytes)**:
- Used for file content (client and server)
- Temporary - exists only during function call
- Prevents overwriting global buffer

**Partial Write Problem**:
```c
write(sock, buffer, strlen(buffer))
```
- May send fewer bytes than requested
- Happens when TCP send buffer is full
- Must check return value and retry if needed
- This project checks and aborts on partial write

---

## Error Handling

### Server-Side Errors

1. **Socket creation failure**:
   ```c
   if (sockfd < 0) perror("ERROR opening socket");
   ```

2. **Bind failure** (port already in use):
   ```c
   if (bind(...) < 0) perror("ERROR on binding");
   ```

3. **Accept failure**:
   ```c
   if (new_sockfd < 0) perror("ERROR on accept");
   ```

4. **Read/Write failures**:
   ```c
   if (n < 0) perror("ERROR reading/writing");
   ```

5. **Client disconnect** (n == 0):
   ```c
   if (n == 0) return 5;  // Exit code
   ```

### Client-Side Errors

1. **Connection refused** (server not running):
   ```c
   if (connect(...) < 0) perror("ERROR connecting");
   ```

2. **Hostname resolution failure**:
   ```c
   if (server == NULL) fprintf(stderr, "ERROR, no such host");
   ```

3. **Server closes connection**:
   ```c
   if (n == 0) printf("Server closed connection");
   ```

---

## Protocol Design

### Message Format

**Menu Request-Response**:
```
Server -> Client: Menu text (with separators)
Client -> Server: "1\n" or "2\n" etc.
Server -> Client: Response data
```

**Session Flow**:
```
1. Server sends menu
2. Client sends choice
3. Server sends response
4. Repeat until client sends "5" (exit)
```

### Synchronization

**Critical**: Client and server must stay synchronized
- Server: WRITE menu, READ choice, WRITE response
- Client: READ menu, WRITE choice, READ response

**Why synchronization matters**:
- TCP is stream-based (no message boundaries)
- Both sides must read/write in same order
- Mismatch causes client to read menu as data or vice versa

---

## Compilation and Execution

### Compilation
```bash
gcc -o server server.c -Wall
gcc -o client client.c -Wall
```

### Running Server
```bash
./server 8080
```
Then choose:
- `1` for multi-process (concurrent clients)
- `2` for FIFO (sequential clients)

### Running Client
```bash
./client localhost 8080
```

Then choose from menu:
- `1` - View date and time
- `2` - List files in data directory
- `3` - View file content (my_data.txt)
- `4` - View session elapsed time
- `5` - Exit

---

## Bug Fixes Applied

### 1. Server Stuck After First Request
**Problem**: Server stopped responding after handling first client request

**Root Cause**: 
- Missing check for partial writes
- `write()` could return less than requested bytes
- Server thought write succeeded and moved to next iteration
- Client still waiting for complete data
- Deadlock: Server waiting for next choice, client waiting for complete response

**Solution**:
```c
if (n < strlen(buffer)){
    perror("Partial write occurred");
    return 0;
}
```

### 2. File Content Disconnection
**Problem**: Server disconnected after sending file content

**Root Cause**:
```c
case 3:
    file_content(...);
    return 0;  // Ends session
```

**Solution**: Changed to `return 1;` to continue session

### 3. File Content Truncation
**Problem**: Only first 256 bytes of file were sent

**Root Cause**: Used global 256-byte buffer

**Solution**: 
- Created local 4096-byte buffer in `file_content()`
- Client also uses 4096-byte buffer in `show_answer()`

### 4. Missing Connection Closure Detection
**Problem**: Client didn't handle server disconnect gracefully

**Solution**: Added check for `n == 0` in all read operations:
```c
if (n == 0){
    printf("Server closed connection\n");
    run = 0;
}
```

### 5. Missing String Header
**Problem**: Implicit function declarations for `strlen()`, `bzero()`

**Solution**: Added `#include <string.h>` to `serverdef.h`

---

## Testing Scenarios

### Test 1: Single Client Session
```bash
# Terminal 1
./server 8080
# Choose option 2 (FIFO)

# Terminal 2
./client localhost 8080
# Try options 1, 2, 3, 4, then 5
```

### Test 2: Multiple Concurrent Clients
```bash
# Terminal 1
./server 8080
# Choose option 1 (Multi-process)

# Terminal 2
./client localhost 8080

# Terminal 3
./client localhost 8080

# Both clients can interact simultaneously
```

### Test 3: Sequential Client Handling
```bash
# Terminal 1
./server 8080
# Choose option 2 (FIFO)

# Terminal 2
./client localhost 8080
# Start interacting

# Terminal 3
./client localhost 8080
# This client waits until Terminal 2 client exits
```

---

## Future Enhancements

1. **Multi-threading**: Use threads instead of processes for better resource usage
2. **Authentication**: Add username/password validation
3. **File upload**: Allow clients to upload files
4. **Encryption**: Use SSL/TLS for secure communication
5. **Configuration file**: Load settings from file instead of command line
6. **Logging**: Record all client activities
7. **Dynamic file selection**: Let client choose which file to view
8. **Better protocol**: Use length-prefixed messages for reliable communication
9. **Connection pooling**: Reuse connections for better performance
10. **Graceful shutdown**: Handle SIGINT to close all connections properly

---

## Project Structure Summary

```
tcp_multi_serv-mult-cli/
├── server.c          # Server entry point with mode selection
├── serverimp.c       # Server implementation (socket, communication)
├── serverdef.h       # Server headers and globals
├── service.c         # Service implementations (date, files, etc.)
├── service.h         # Service function prototypes
├── client.c          # Client entry point
├── clientdef.h       # Client implementation
├── data/
│   └── my_data.txt   # Sample file for testing
├── server            # Compiled server executable
├── client            # Compiled client executable
└── README.md         # This documentation
```

---

## Conclusion

This project demonstrates fundamental concepts in network programming:
- TCP socket programming in C
- Client-server architecture
- Process management with fork()
- Concurrent vs sequential client handling
- Protocol design and synchronization
- Error handling and resource management

The dual-mode server design allows comparison of different architectures and understanding their trade-offs in real-world scenarios.
