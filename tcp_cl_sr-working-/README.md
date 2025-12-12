# TCP Multi-Server Multi-Client Application

<div align="center">

![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![GTK](https://img.shields.io/badge/GTK-7FE719?style=for-the-badge&logo=gtk&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
![OpenSSL](https://img.shields.io/badge/OpenSSL-721412?style=for-the-badge&logo=openssl&logoColor=white)

A robust TCP client-server application with **secure authentication**, **multiple server modes**, and a modern **GTK+3 graphical interface**.

[Features](#features) • [Installation](#installation) • [Usage](#usage) • [Documentation](#documentation)

</div>

---

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Usage](#usage)
  - [Server](#server)
  - [CLI Client](#cli-client)
  - [GUI Client](#gui-client)
- [Services](#services)
- [Authentication](#authentication)
- [Project Structure](#project-structure)
- [Building](#building)
- [Testing](#testing)
- [Screenshots](#screenshots)
- [Contributing](#contributing)
- [License](#license)

---

## 🌟 Overview

This is a comprehensive TCP-based client-server application built in C, featuring secure user authentication, multiple server operation modes, and both command-line and graphical user interfaces. The application demonstrates advanced network programming concepts including socket programming, process management, secure password handling, and GUI development with GTK+3.

### Key Highlights

- 🔐 **Secure Authentication**: SHA-256 password hashing with unique salts
- 🚀 **Multiple Server Modes**: Multi-process, FIFO, and MONO modes
- 🖥️ **Dual Interface**: Both CLI and modern GTK+3 GUI clients
- 📁 **File Services**: Remote file listing and content retrieval
- ⏱️ **Session Management**: Track connection time and session tokens
- 🔒 **Thread-Safe**: Mutex-protected user database

---

## ✨ Features

### Server Features

#### Three Operation Modes
- **Multi-Process Mode** 🔄
  - Concurrent client handling using `fork()`
  - Each client gets a dedicated process
  - Ideal for production environments with multiple simultaneous users

- **FIFO Mode** 📋
  - Sequential client processing
  - One client at a time
  - Lower resource usage

- **MONO Mode** 🔌
  - Single client, no forking
  - Ideal for testing and debugging

#### Security
- User registration and login system
- SHA-256 password hashing
- Unique salt generation per user
- Session token management
- Password validation (minimum 6 characters)
- Username validation (alphanumeric, underscore, hyphen)

#### Services
1. **Date & Time**: Display current server date and time
2. **Directory Listing**: List files in the server's data directory
3. **File Content**: Read and transfer file contents
4. **Session Time**: Show elapsed connection time

### Client Features

#### CLI Client
- Interactive command-line interface
- Hidden password input (no echo)
- Full service access
- Lightweight and fast

#### GUI Client (GTK+3)
- Modern, intuitive graphical interface
- Three-page navigation:
  - Connection configuration
  - User authentication (login/register)
  - Service menu with real-time response display
- File selection dialogs
- Scrollable response viewer
- Clean, professional design

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    CLIENT LAYER                         │
├─────────────────────────────────────────────────────────┤
│  CLI Client           │         GUI Client (GTK+3)      │
│  (Terminal-based)     │         (Graphical Interface)   │
└───────────┬───────────┴──────────────┬──────────────────┘
            │                          │
            └──────────┬───────────────┘
                       │ TCP Connection
                       │
┌──────────────────────▼──────────────────────────────────┐
│                   SERVER LAYER                           │
├─────────────────────────────────────────────────────────┤
│                                                          │
│  ┌────────────────────────────────────────────┐         │
│  │     Authentication System (auth.c)         │         │
│  │  - SHA-256 Hashing                         │         │
│  │  - Salt Generation                         │         │
│  │  - Session Management                      │         │
│  └────────────────────────────────────────────┘         │
│                                                          │
│  ┌────────────────────────────────────────────┐         │
│  │     Service Layer (service.c)              │         │
│  │  - Date/Time Service                       │         │
│  │  - File Operations                         │         │
│  │  - Directory Listing                       │         │
│  └────────────────────────────────────────────┘         │
│                                                          │
│  ┌────────────────────────────────────────────┐         │
│  │     Server Implementation (serverimp.c)    │         │
│  │  - Socket Management                       │         │
│  │  - Connection Handling                     │         │
│  │  - Request Processing                      │         │
│  └────────────────────────────────────────────┘         │
│                                                          │
│  Server Modes: Multi-Process | FIFO | MONO              │
└─────────────────────────────────────────────────────────┘
```

---

## 📦 Prerequisites

### Required Libraries

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential libssl-dev libgtk-3-dev pkg-config

# Fedora/RHEL/CentOS
sudo dnf install gcc openssl-devel gtk3-devel pkg-config

# Arch Linux
sudo pacman -S base-devel openssl gtk3 pkg-config
```

### Dependencies
- **GCC**: C compiler
- **OpenSSL**: Cryptographic functions (libssl, libcrypto)
- **GTK+ 3.0**: GUI development library
- **pthread**: POSIX threads (included with GCC)
- **pkg-config**: For managing library compile/link flags

### Verify Installation

```bash
# Check GCC
gcc --version

# Check OpenSSL
openssl version

# Check GTK+3
pkg-config --modversion gtk+-3.0

# Should output something like: 3.24.x
```

---

## 🚀 Installation

### Clone the Repository

```bash
git clone https://github.com/thamer-elhani/tcp_cl_sr-working-.git
cd tcp_cl_sr-working-
```

### Build All Components

```bash
make
```

This will build:
- `server` - The server executable
- `client` - The CLI client executable
- `gui_client` - The GUI client executable

### Build Individual Components

```bash
make server       # Build server only
make client       # Build CLI client only
make gui_client   # Build GUI client only
```

---

## 🎯 Quick Start

### 1. Start the Server

```bash
./server 8080
```

Select a mode when prompted:
```
========================================
      TCP SERVER - MODE SELECTION
========================================
1. Multi-process (Concurrent clients)
2. FIFO/Sequential (One client at a time)
3. MONO (Single client, no fork)
========================================
Enter your choice: 1
```

### 2. Launch the GUI Client

```bash
./gui_client
```

Or use the CLI client:

```bash
./client localhost 8080
```

### 3. Create an Account

1. In the GUI, enter hostname: `localhost` and port: `8080`
2. Click "Connect"
3. Choose "Register" on the authentication screen
4. Enter username and password (min 6 characters)
5. Click "Register"

### 4. Use Services

Once authenticated, use the service buttons to:
- View server date/time
- List files in data directory
- Read file contents
- Check session duration

---

## 📖 Usage

### Server

#### Start Server

```bash
./server <port>
```

**Example:**
```bash
./server 8080
```

#### Server Modes

| Mode | Description | Use Case |
|------|-------------|----------|
| **Multi-Process** | Concurrent clients using fork() | Production with multiple users |
| **FIFO** | Sequential client handling | Testing, single-user scenarios |
| **MONO** | Single client, no fork | Debugging, development |

#### Makefile Shortcuts

```bash
make run-server-multi    # Start in multi-process mode
make run-server-fifo     # Start in FIFO mode
```

### CLI Client

#### Connect to Server

```bash
./client <hostname> <port>
```

**Example:**
```bash
./client localhost 8080
./client 192.168.1.100 8080
```

#### Authentication

On first connection, you'll see:
```
========================================
         AUTHENTICATION
========================================
1. Register new account
2. Login with existing account
========================================
Enter your choice (1-2):
```

#### Service Menu

After authentication:
```
========================================
         SERVER MENU
========================================
1. Show date and time
2. List directory files
3. Display file content (specify path)
4. Show session elapsed time
5. Exit
========================================
Enter your choice:
```

### GUI Client

#### Launch

```bash
./gui_client
```

Or:
```bash
make run-gui
```

#### Connection Screen

1. **Hostname**: Enter server hostname (default: `localhost`)
2. **Port**: Enter server port (default: `8080`)
3. Click **Connect**

#### Authentication Screen

**For New Users:**
- Enter desired username
- Enter password (min 6 chars)
- Click **Register**

**For Existing Users:**
- Enter username
- Enter password
- Click **Login**

#### Main Menu

Five service buttons available:

1. **Show Date and Time** - Displays current server timestamp
2. **List Directory Files** - Shows files in server's data folder
3. **Display File Content** - Opens dialog to enter filename, then displays content
4. **Show Session Time** - Shows how long you've been connected
5. **Disconnect** - Closes connection and returns to connection screen

---

## 🛠️ Services

### 1. Date and Time Service

Returns the current date and time from the server.

**Format:** `YYYY-MM-DD HH:MM:SS`

**Example Response:**
```
2025-12-12 20:15:30
```

### 2. List Directory Files

Lists all files in the server's `./data` directory.

**Example Response:**
```
.
..
my_data.txt
credentials.dat
```

### 3. Display File Content

Reads and displays the content of a file from the `./data` directory.

**Usage:**
- CLI: Enter filename when prompted
- GUI: Dialog box appears for filename input

**Example:**
```
Filename: my_data.txt
```

**Response:** Full content of the file

### 4. Session Time

Displays elapsed time since connection was established.

**Example Response:**
```
Current session has been active for 145 seconds.
```

---

## 🔐 Authentication

### Registration

**Requirements:**
- Unique username (3-64 characters)
- Valid characters: alphanumeric, underscore (_), hyphen (-)
- Password: minimum 6 characters

**Process:**
1. Username validation
2. Password validation
3. Salt generation (16 bytes)
4. SHA-256 hashing with salt
5. Store in `data/credentials.dat`

### Login

**Process:**
1. Username lookup
2. Retrieve stored salt
3. Hash provided password with salt
4. Compare with stored hash
5. Generate session token on success

### Security Features

- ✅ Passwords never stored in plain text
- ✅ Unique salt per user
- ✅ SHA-256 cryptographic hashing
- ✅ Session token management
- ✅ Thread-safe operations (mutex locks)
- ✅ Password input hidden in both CLI and GUI

### Credentials Storage

File: `data/credentials.dat`
- Automatically created on first registration
- Binary format
- Contains: username, password hash, salt

---

## 📁 Project Structure

```
tcp_cl_sr-working-/
├── 📄 README.md              # Main documentation (this file)
├── 📄 README_GUI.md          # Detailed GUI documentation
├── 📄 Makefile               # Build configuration
├── 📄 .gitignore             # Git ignore rules
│
├── 🖥️ SERVER
│   ├── server.c              # Server main program
│   ├── serverdef.h           # Server definitions and prototypes
│   ├── serverimp.c           # Server implementation
│   ├── auth.c                # Authentication implementation
│   ├── auth.h                # Authentication header
│   ├── service.c             # Service implementations
│   └── service.h             # Service header
│
├── 💻 CLI CLIENT
│   ├── client.c              # CLI client main program
│   └── clientdef.h           # Client definitions
│
├── 🎨 GUI CLIENT
│   ├── gui_client.c          # GUI client implementation
│   └── gui_client.h          # GUI client header
│
└── 📂 data/
    ├── credentials.dat       # User credentials (auto-generated)
    └── my_data.txt           # Sample data file
```

---

## 🔨 Building

### Build All

```bash
make
```

Output:
```
=========================================
Build completed successfully!
=========================================
Server executable: ./server
Client executable: ./client
GUI Client executable: ./gui_client
```

### Build Options

```bash
make all          # Build everything (default)
make server       # Build server only
make client       # Build CLI client only
make gui_client   # Build GUI client only
make clean        # Remove build artifacts
make distclean    # Remove all generated files
make check        # Check build status
make help         # Show help menu
```

### Compiler Flags

- **CFLAGS**: `-Wall -Wextra -g`
  - Full warnings enabled
  - Debug symbols included

- **LDFLAGS**: `-lssl -lcrypto -lpthread`
  - OpenSSL libraries
  - POSIX threads

- **GTK_CFLAGS**: From `pkg-config --cflags gtk+-3.0`
- **GTK_LDFLAGS**: From `pkg-config --libs gtk+-3.0`

---

## 🧪 Testing

### Automated Test

```bash
make test
```

This runs a basic test scenario with server and client.

### Manual Testing

#### Test Multi-Process Server

**Terminal 1 (Server):**
```bash
./server 8080
1  # Select multi-process mode
```

**Terminal 2 (Client 1):**
```bash
./gui_client
```

**Terminal 3 (Client 2):**
```bash
./gui_client
```

**Terminal 4 (CLI Client):**
```bash
./client localhost 8080
```

All clients should be able to connect and interact simultaneously.

#### Test FIFO Server

```bash
# Terminal 1
./server 8080
2  # Select FIFO mode

# Terminal 2
./client localhost 8080
# Try connecting a second client - it will wait until first disconnects
```

### Adding Test Data

```bash
# Create test files in data directory
echo "Hello, World!" > data/hello.txt
echo "Test file content" > data/test.txt

# Test file reading service
./gui_client
# Use "Display File Content" and enter: hello.txt
```

---

## 📸 Screenshots

### CLI Client
```
========================================
         AUTHENTICATION
========================================
1. Register new account
2. Login with existing account
========================================
Enter your choice (1-2): 1
Username: john_doe
Password: ******
[AUTH] Registration successful! You are now logged in.

========================================
         SERVER MENU
========================================
1. Show date and time
2. List directory files
3. Display file content (specify path)
4. Show session elapsed time
5. Exit
========================================
Enter your choice: 1
2025-12-12 20:15:30
```

### GUI Client

**Connection Screen:**
- Clean input fields for hostname and port
- Status indicator
- Professional layout

**Authentication Screen:**
- Username and password fields
- Login and Register buttons
- Real-time status messages

**Main Menu:**
- Five clearly labeled service buttons
- Large scrollable text area for responses
- Disconnect button

---

## 🔧 Troubleshooting

### Common Issues

#### 1. GUI Client Won't Compile

**Error:** `Package gtk+-3.0 was not found`

**Solution:**
```bash
# Install GTK+3 development libraries
sudo apt-get install libgtk-3-dev pkg-config

# Verify installation
pkg-config --modversion gtk+-3.0
```

#### 2. Connection Refused

**Symptoms:** Client can't connect to server

**Solutions:**
- ✅ Verify server is running: `ps aux | grep server`
- ✅ Check port matches: Server and client use same port
- ✅ Check firewall: `sudo ufw allow 8080`
- ✅ Try localhost: Use `localhost` or `127.0.0.1`

#### 3. Authentication Failed

**Symptoms:** Login fails with valid credentials

**Solutions:**
- ✅ Check `data/credentials.dat` exists
- ✅ Verify username is correct (case-sensitive)
- ✅ Ensure password is at least 6 characters
- ✅ Try registering a new account

#### 4. Port Already in Use

**Error:** `ERROR on binding`

**Solution:**
```bash
# Find process using port 8080
lsof -ti:8080

# Kill the process
lsof -ti:8080 | xargs kill -9

# Or use a different port
./server 9000
```

#### 5. File Not Found

**Error:** When reading file content

**Solutions:**
- ✅ File must be in `data/` directory
- ✅ Enter filename only, not path: `my_data.txt` not `data/my_data.txt`
- ✅ Check file exists: `ls data/`
- ✅ Check file permissions: `ls -la data/`

---

## 🚀 Advanced Usage

### Remote Connections

#### Server on Different Machine

```bash
# On server machine (192.168.1.100)
./server 8080

# On client machine
./gui_client
# Enter hostname: 192.168.1.100
# Enter port: 8080
```

### Custom Ports

```bash
# Server
./server 9000

# CLI Client
./client localhost 9000

# GUI Client
# Enter port: 9000 in the connection screen
```

### Adding Custom Data Files

```bash
# Add files for clients to access
echo "Custom content" > data/custom.txt
echo "More data" > data/info.txt

# Clients can now read these files using service option 3
```

### Viewing Server Logs

```bash
# Run server with output redirection
./server 8080 2>&1 | tee server.log

# View logs in real-time
tail -f server.log
```

---

## 📊 Performance & Limits

### Server Capacity

| Parameter | Value | Defined In |
|-----------|-------|------------|
| Max Users | 100 | `auth.h` - `MAX_USERS` |
| Max Username Length | 64 chars | `auth.h` - `MAX_USERNAME` |
| Max Password Length | 128 chars | `auth.h` - `MAX_PASSWORD` |
| Min Password Length | 6 chars | `auth.h` - `MIN_PASSWORD_LENGTH` |
| Session Timeout | 3600 sec (1 hour) | `auth.h` - `SESSION_TIMEOUT` |
| Buffer Size | 256 bytes | `serverdef.h` - `MAX_BUFFER` |

### Memory Usage

- **Multi-Process**: ~2-5 MB per client process
- **FIFO**: ~1-2 MB total
- **MONO**: ~1-2 MB total

### Recommended Specs

- **CPU**: Any modern processor
- **RAM**: 512 MB minimum, 1 GB recommended
- **Disk**: 50 MB for application + space for data files
- **Network**: 100 Kbps per client minimum

---

## 🤝 Contributing

Contributions are welcome! Here's how you can help:

### Reporting Bugs

1. Check existing issues
2. Create a new issue with:
   - Clear description
   - Steps to reproduce
   - Expected vs actual behavior
   - System information (OS, GCC version, etc.)

### Suggesting Features

1. Open an issue with the `enhancement` label
2. Describe the feature and use case
3. Explain why it would be beneficial

### Pull Requests

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Make your changes
4. Test thoroughly
5. Commit: `git commit -m 'Add amazing feature'`
6. Push: `git push origin feature/amazing-feature`
7. Open a Pull Request

### Code Style

- 4-space indentation
- Clear, descriptive variable names
- Comments for complex logic
- Error checking for all system calls
- Follow existing code structure

---

## 📝 Development

### Adding New Services

1. **Add service function** in `service.c`:
```c
void my_new_service(char *buffer, int max_buffer) {
    // Your implementation
}
```

2. **Add declaration** in `service.h`:
```c
void my_new_service(char *buffer, int max_buffer);
```

3. **Update server** in `serverimp.c`:
```c
case 6:
    my_new_service(buffer, MAX_BUFFER);
    write(sock, buffer, strlen(buffer));
    return 1;
```

4. **Update clients** to add menu option for new service

### Debugging

```bash
# Compile with debug symbols (already included)
make clean && make

# Run with GDB
gdb ./server
gdb ./client
gdb ./gui_client

# Use valgrind for memory leaks
valgrind --leak-check=full ./server 8080
```

---

## 📄 License

This project is created for educational purposes. Feel free to use, modify, and distribute as needed.

---

## 👥 Authors

**TCP Server-Client Project Team**

- Secure authentication system implementation
- Multi-mode server architecture
- GTK+3 GUI development
- Documentation and testing

---

## 🙏 Acknowledgments

- OpenSSL for cryptographic functions
- GTK+ team for the excellent GUI library
- The open-source community

---

## 📞 Support

### Getting Help

1. Check the [Troubleshooting](#troubleshooting) section
2. Read [README_GUI.md](README_GUI.md) for detailed GUI documentation
3. Review the code comments
4. Open an issue on GitHub

### Useful Commands Reference

```bash
# Build
make                      # Build all components
make clean               # Clean build artifacts

# Run
./server 8080            # Start server
./client localhost 8080  # Start CLI client
./gui_client            # Start GUI client

# Test
make test               # Run automated tests
make check              # Check build status

# Help
make help               # Show all make targets
```

---

## 📈 Roadmap

Future enhancements could include:

- [ ] SSL/TLS encryption for secure communication
- [ ] Database integration (SQLite/PostgreSQL)
- [ ] File upload functionality
- [ ] Chat feature between clients
- [ ] Web-based client interface
- [ ] Configuration file support
- [ ] Logging system
- [ ] Rate limiting
- [ ] IPv6 support
- [ ] Windows compatibility

---

## 📚 Additional Resources

- [GTK+ Documentation](https://docs.gtk.org/gtk3/)
- [OpenSSL Documentation](https://www.openssl.org/docs/)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [POSIX Threads Programming](https://computing.llnl.gov/tutorials/pthreads/)

---

<div align="center">

### ⭐ Star this repository if you found it helpful!

Made with ❤️ using C, GTK+3, and OpenSSL

</div>
