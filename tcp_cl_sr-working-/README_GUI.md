# TCP Multi-Server Multi-Client Application - GUI Version

## Overview

This project is a comprehensive TCP client-server application with support for both command-line and graphical user interface (GUI) clients. The application features secure authentication, multiple server modes, and various network services.

## Features

### Server Features
- **Multiple Server Modes:**
  - **Multi-Process Mode**: Handles multiple concurrent clients using fork()
  - **FIFO Mode**: Handles clients sequentially, one at a time
  - **MONO Mode**: Single client, no forking
  
- **Secure Authentication System:**
  - User registration and login
  - Password hashing with SHA-256
  - Salt generation for enhanced security
  - Session management with tokens
  
- **Services:**
  - Date and time display
  - Directory file listing (./data directory)
  - File content retrieval
  - Session elapsed time tracking

### Client Features
- **CLI Client**: Traditional command-line interface
- **GUI Client**: Modern graphical interface built with GTK+3
  - Connection management
  - User authentication (login/register)
  - Interactive service menu
  - Real-time response display

## Prerequisites

### Required Libraries
```bash
# On Ubuntu/Debian
sudo apt-get install build-essential libssl-dev libgtk-3-dev pkg-config

# On Fedora/RHEL
sudo dnf install gcc openssl-devel gtk3-devel pkg-config

# On Arch Linux
sudo pacman -S base-devel openssl gtk3 pkg-config
```

### Dependencies
- **GCC**: C compiler
- **OpenSSL**: For cryptographic functions (libssl, libcrypto)
- **GTK+ 3.0**: For GUI client
- **pthread**: POSIX threads (usually included with gcc)

## Building the Project

### Build All Components
```bash
make
```
This builds the server, CLI client, and GUI client.

### Build Individual Components
```bash
make server       # Build server only
make client       # Build CLI client only
make gui_client   # Build GUI client only
```

### Clean Build Artifacts
```bash
make clean        # Remove executables and object files
make distclean    # Remove all generated files including credentials
```

## Running the Application

### 1. Start the Server

```bash
./server <port>
```

When prompted, select a server mode:
1. Multi-process (Concurrent clients)
2. FIFO/Sequential (One client at a time)
3. MONO (Single client, no fork)

Example:
```bash
./server 8080
# Then enter: 1 (for multi-process mode)
```

Or use the Makefile shortcuts:
```bash
make run-server-multi    # Start in multi-process mode on port 8080
make run-server-fifo     # Start in FIFO mode on port 8080
```

### 2. Connect with GUI Client (Recommended)

```bash
./gui_client
```

Or:
```bash
make run-gui
```

#### GUI Client Usage:
1. **Connection Screen:**
   - Enter hostname (default: localhost)
   - Enter port (default: 8080)
   - Click "Connect"

2. **Authentication Screen:**
   - **For New Users:**
     - Enter username
     - Enter password (minimum 6 characters)
     - Click "Register"
   
   - **For Existing Users:**
     - Enter username
     - Enter password
     - Click "Login"

3. **Main Menu:**
   - **Show Date and Time**: Display current server date/time
   - **List Directory Files**: Show files in server's data directory
   - **Display File Content**: View content of a specific file
     - A dialog will prompt for the filename
     - Enter filename (e.g., "my_data.txt")
   - **Show Session Time**: Display elapsed session time
   - **Disconnect**: Close connection and return to connection screen

### 3. Connect with CLI Client (Alternative)

```bash
./client <hostname> <port>
```

Example:
```bash
./client localhost 8080
```

Or:
```bash
make run-client
```

Follow the on-screen prompts for authentication and service selection.

## Project Structure

```
tcp_cl_sr-working-/
├── server.c              # Server main program
├── serverdef.h           # Server definitions
├── serverimp.c           # Server implementation
├── client.c              # CLI client main program
├── clientdef.h           # Client definitions
├── gui_client.c          # GUI client implementation
├── gui_client.h          # GUI client header
├── auth.c                # Authentication implementation
├── auth.h                # Authentication header
├── service.c             # Service implementations
├── service.h             # Service header
├── Makefile              # Build configuration
├── README_GUI.md         # This file
└── data/                 # Data directory
    ├── credentials.dat   # User credentials (auto-generated)
    └── my_data.txt       # Sample data file
```

## Authentication System

### First Time Setup
On first run, the authentication system will create a `data/credentials.dat` file to store user credentials securely.

### User Registration
1. Choose "Register" in the GUI or option "1" in CLI
2. Enter a unique username (alphanumeric, underscore, hyphen allowed)
3. Enter a password (minimum 6 characters)
4. Password is securely hashed with SHA-256 and a unique salt

### User Login
1. Choose "Login" in the GUI or option "2" in CLI
2. Enter your username and password
3. Credentials are verified against stored hash

### Security Features
- Passwords are never stored in plain text
- Each password has a unique salt
- SHA-256 hashing algorithm
- Session tokens for authenticated connections
- Password input is hidden in both CLI and GUI

## Services Explained

### 1. Show Date and Time
Returns the current date and time from the server.

### 2. List Directory Files
Lists all files in the server's `./data` directory.

### 3. Display File Content
Prompts for a filename and displays its content. Files must be located in the `./data` directory on the server.

Example files you can read:
- `my_data.txt` (included in the project)

### 4. Show Session Time
Displays how long the current session has been active.

## Testing

### Automated Test
```bash
make test
```
This runs a basic test scenario with the server and client.

### Manual Testing with Multiple Clients

Terminal 1 (Server):
```bash
./server 8080
1  # Select multi-process mode
```

Terminal 2 (GUI Client 1):
```bash
./gui_client
```

Terminal 3 (GUI Client 2):
```bash
./gui_client
```

Terminal 4 (CLI Client):
```bash
./client localhost 8080
```

## Troubleshooting

### GUI Client Won't Compile
Make sure GTK+ 3 development libraries are installed:
```bash
pkg-config --modversion gtk+-3.0
```
This should return a version number (e.g., 3.24.x).

### Connection Refused
- Ensure the server is running
- Check that the port number matches
- Verify firewall settings allow the connection
- Try using `localhost` or `127.0.0.1` as hostname

### Authentication Failed
- Check username and password are correct
- For registration, ensure username is unique
- Password must be at least 6 characters
- Check that `data/credentials.dat` exists and is readable

### Port Already in Use
```bash
# Find and kill process using port 8080
lsof -ti:8080 | xargs kill -9
```

### Server Won't Start
- Ensure port number is valid (1024-65535 for non-root users)
- Check that the port is not already in use
- Verify OpenSSL libraries are installed

## Advanced Usage

### Custom Port
```bash
./server 9000          # Server on port 9000
./gui_client           # GUI: Enter 9000 in port field
./client localhost 9000 # CLI with custom port
```

### Remote Server
```bash
# Server on machine 192.168.1.100
./server 8080

# GUI Client from another machine
./gui_client
# Enter 192.168.1.100 as hostname in GUI

# CLI Client from another machine
./client 192.168.1.100 8080
```

### Adding Data Files
Place files in the `data/` directory for clients to access:
```bash
echo "Hello, World!" > data/hello.txt
```
Then use "Display File Content" service and enter `hello.txt`.

## Development

### Compile with Debug Symbols
The Makefile includes `-g` flag by default for debugging.

### Adding New Services
1. Add service function in `service.c` and declaration in `service.h`
2. Update server case handling in `serverimp.c`
3. Add button/menu option in GUI client (`gui_client.c`)
4. Update CLI client menu in `clientdef.h`

### Code Style
- 4-space indentation
- Clear variable names
- Comments for complex logic
- Error checking for all system calls

## Performance Considerations

### Server Modes
- **Multi-Process**: Best for concurrent clients, uses more memory (one process per client)
- **FIFO**: Best for sequential operations, low memory overhead
- **MONO**: Best for single client testing

### Limits
- Maximum users: 100 (defined in `auth.h` as `MAX_USERS`)
- Maximum username length: 64 characters
- Maximum password length: 128 characters
- Buffer size: 256 bytes (standard operations)
- File content buffer: 8192 bytes (GUI), 1024 bytes (CLI)

## License

This project is for educational purposes.

## Authors

TCP Server-Client Project Team

## Changelog

### Version 2.0 (December 2025)
- Added GTK+ 3.0 GUI client
- Enhanced Makefile with GUI support
- Improved documentation

### Version 1.0
- Initial release with CLI client and server
- Authentication system
- Multiple server modes
- Basic services

## Support

For issues or questions:
1. Check the troubleshooting section
2. Verify all dependencies are installed
3. Review the build output for errors
4. Check server logs for connection issues

## Screenshots

### GUI Client - Connection Screen
```
┌─────────────────────────────────┐
│   TCP Client Connection         │
│                                 │
│  Hostname: [localhost      ]    │
│  Port:     [8080           ]    │
│                                 │
│        [ Connect ]              │
│  Status: Ready to connect       │
└─────────────────────────────────┘
```

### GUI Client - Authentication
```
┌─────────────────────────────────┐
│   Authentication                │
│                                 │
│  Username: [_____________ ]     │
│  Password: [*************]      │
│                                 │
│  [ Login ]  [ Register ]        │
└─────────────────────────────────┘
```

### GUI Client - Main Menu
```
┌─────────────────────────────────┐
│   Server Menu                   │
│                                 │
│  [ Show Date and Time      ]    │
│  [ List Directory Files    ]    │
│  [ Display File Content    ]    │
│  [ Show Session Time       ]    │
│  [ Disconnect              ]    │
│                                 │
│  ┌──────────────────────────┐  │
│  │ Response appears here... │  │
│  │                          │  │
│  └──────────────────────────┘  │
└─────────────────────────────────┘
```

## Quick Reference

```bash
# Build
make                      # Build all
make gui_client          # Build GUI only

# Run
./server 8080            # Start server
./gui_client            # Start GUI client
./client localhost 8080  # Start CLI client

# Clean
make clean              # Remove builds
make distclean          # Remove all generated files
```
