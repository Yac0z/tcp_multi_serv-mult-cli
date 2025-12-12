# Contributing to TCP Multi-Server Multi-Client Application

First off, thank you for considering contributing to this project! It's people like you that make this project better for everyone.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
  - [Reporting Bugs](#reporting-bugs)
  - [Suggesting Enhancements](#suggesting-enhancements)
  - [Pull Requests](#pull-requests)
- [Development Setup](#development-setup)
- [Coding Standards](#coding-standards)
- [Testing Guidelines](#testing-guidelines)
- [Commit Messages](#commit-messages)

## Code of Conduct

This project and everyone participating in it is governed by basic principles of respect and collaboration. By participating, you are expected to uphold these principles.

## How Can I Contribute?

### Reporting Bugs

Before creating bug reports, please check existing issues to avoid duplicates. When you create a bug report, include as many details as possible:

**Bug Report Template:**

```markdown
**Description**
A clear and concise description of the bug.

**To Reproduce**
Steps to reproduce the behavior:
1. Start server with '...'
2. Connect client with '...'
3. Click on '....'
4. See error

**Expected Behavior**
What you expected to happen.

**Actual Behavior**
What actually happened.

**Environment:**
- OS: [e.g., Ubuntu 22.04]
- GCC Version: [e.g., 11.3.0]
- GTK Version: [e.g., 3.24.33]
- OpenSSL Version: [e.g., 3.0.2]

**Screenshots**
If applicable, add screenshots to help explain the problem.

**Additional Context**
Any other context about the problem.
```

### Suggesting Enhancements

Enhancement suggestions are tracked as GitHub issues. When creating an enhancement suggestion, please include:

**Enhancement Template:**

```markdown
**Feature Description**
Clear and concise description of the feature.

**Use Case**
Why would this feature be useful? What problem does it solve?

**Proposed Implementation**
If you have ideas about how to implement this, describe them here.

**Alternatives Considered**
What alternative solutions or features have you considered?

**Additional Context**
Any other context, mockups, or examples.
```

### Pull Requests

1. **Fork the Repository**
   ```bash
   # Fork on GitHub, then clone your fork
   git clone https://github.com/YOUR_USERNAME/tcp_cl_sr-working-.git
   cd tcp_cl_sr-working-
   ```

2. **Create a Branch**
   ```bash
   git checkout -b feature/amazing-feature
   # or
   git checkout -b fix/bug-fix
   ```

3. **Make Your Changes**
   - Write clear, commented code
   - Follow the coding standards (see below)
   - Add tests if applicable
   - Update documentation

4. **Test Your Changes**
   ```bash
   make clean
   make
   make test
   ```

5. **Commit Your Changes**
   ```bash
   git add .
   git commit -m "Add: Brief description of changes"
   ```

6. **Push to Your Fork**
   ```bash
   git push origin feature/amazing-feature
   ```

7. **Create Pull Request**
   - Go to the original repository
   - Click "New Pull Request"
   - Select your branch
   - Fill in the PR template

**Pull Request Template:**

```markdown
## Description
Brief description of what this PR does.

## Type of Change
- [ ] Bug fix (non-breaking change which fixes an issue)
- [ ] New feature (non-breaking change which adds functionality)
- [ ] Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] Documentation update

## Testing
Describe the tests you ran and how to reproduce them.

## Checklist
- [ ] My code follows the style guidelines of this project
- [ ] I have performed a self-review of my own code
- [ ] I have commented my code, particularly in hard-to-understand areas
- [ ] I have made corresponding changes to the documentation
- [ ] My changes generate no new warnings
- [ ] I have tested my changes thoroughly
- [ ] Any dependent changes have been merged and published
```

## Development Setup

### Prerequisites

```bash
# Install development tools
sudo apt-get install build-essential libssl-dev libgtk-3-dev pkg-config git

# Verify installation
gcc --version
pkg-config --modversion gtk+-3.0
openssl version
```

### Building

```bash
# Clone and build
git clone https://github.com/thamer-elhani/tcp_cl_sr-working-.git
cd tcp_cl_sr-working-
make

# Run tests
make test
```

### Project Structure

```
tcp_cl_sr-working-/
├── server.c, serverdef.h, serverimp.c    # Server components
├── client.c, clientdef.h                 # CLI client
├── gui_client.c, gui_client.h           # GUI client
├── auth.c, auth.h                       # Authentication
├── service.c, service.h                 # Services
├── Makefile                             # Build configuration
└── data/                                # Data directory
```

## Coding Standards

### General Guidelines

1. **Indentation**: Use 4 spaces (no tabs)
2. **Line Length**: Aim for 80-100 characters, maximum 120
3. **Naming Conventions**:
   - Functions: `snake_case` (e.g., `connect_to_server`)
   - Variables: `snake_case` (e.g., `user_count`)
   - Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_BUFFER`)
   - Structs: `PascalCase` (e.g., `AppWidgets`)

### Code Style Example

```c
// Good
int authenticate_user(const char *username, const char *password) {
    if (username == NULL || password == NULL) {
        return -1;
    }
    
    // Validate username
    if (!validate_username(username)) {
        fprintf(stderr, "Invalid username\n");
        return -2;
    }
    
    // Process authentication
    return process_auth(username, password);
}

// Bad
int authenticateUser(const char* username,const char* password){
if(username==NULL||password==NULL)return -1;
if(!validate_username(username)){fprintf(stderr,"Invalid username\n");return -2;}
return process_auth(username,password);}
```

### Comments

```c
// Single-line comments for brief explanations

/*
 * Multi-line comments for detailed explanations
 * of complex algorithms or functions
 */

/**
 * Function documentation
 * @param username - The username to validate
 * @return 0 on success, -1 on failure
 */
```

### Error Handling

```c
// Always check return values
int result = some_function();
if (result < 0) {
    perror("ERROR: Function failed");
    return -1;
}

// Use descriptive error messages
fprintf(stderr, "[AUTH ERROR] Failed to authenticate user: %s\n", username);
```

### Memory Management

```c
// Always free allocated memory
char *buffer = malloc(256);
if (buffer == NULL) {
    return -1;
}

// ... use buffer ...

free(buffer);
buffer = NULL;  // Good practice
```

## Testing Guidelines

### Manual Testing Checklist

Before submitting a PR, test:

- [ ] Server starts in all three modes
- [ ] CLI client can connect and authenticate
- [ ] GUI client can connect and authenticate
- [ ] All services work correctly
- [ ] Multiple concurrent clients (multi-process mode)
- [ ] Sequential clients (FIFO mode)
- [ ] Error handling (invalid input, connection failures)
- [ ] Memory leaks (use valgrind)

### Testing Commands

```bash
# Build and basic test
make clean && make && make test

# Memory leak check
valgrind --leak-check=full ./server 8080

# Test multi-process mode
# Terminal 1
./server 8080
1  # Multi-process mode

# Terminal 2 & 3
./client localhost 8080

# Test file operations
echo "Test content" > data/test.txt
./gui_client  # Try reading test.txt
```

## Commit Messages

### Format

```
Type: Brief description (50 chars or less)

Detailed explanation of the change, if needed.
Include motivation and contrast with previous behavior.

Fixes #123
```

### Types

- **Add**: New feature or functionality
- **Fix**: Bug fix
- **Update**: Update existing feature
- **Remove**: Remove feature or code
- **Refactor**: Code restructuring without functionality change
- **Docs**: Documentation changes
- **Test**: Adding or updating tests
- **Style**: Code style/formatting changes

### Examples

```bash
# Good commit messages
git commit -m "Add: Session timeout configuration option"
git commit -m "Fix: Memory leak in authentication module"
git commit -m "Update: Improve error handling in GUI client"
git commit -m "Docs: Add troubleshooting section to README"

# Bad commit messages
git commit -m "fixed stuff"
git commit -m "WIP"
git commit -m "asdfasdf"
```

## Areas That Need Contribution

### High Priority

- [ ] Unit tests for authentication module
- [ ] Integration tests for server-client communication
- [ ] SSL/TLS encryption support
- [ ] Configuration file support
- [ ] Better error messages and logging

### Medium Priority

- [ ] File upload functionality
- [ ] Database integration (SQLite)
- [ ] Chat feature between clients
- [ ] Performance optimizations
- [ ] IPv6 support

### Low Priority

- [ ] Web-based client interface
- [ ] Windows compatibility
- [ ] Multi-language support
- [ ] Themes for GUI client
- [ ] Plugin system

## Questions?

If you have questions about contributing, feel free to:

1. Open an issue with the "question" label
2. Check existing documentation
3. Review closed issues and PRs

## Recognition

Contributors will be recognized in:
- README.md contributors section
- Release notes
- Project documentation

Thank you for contributing! 🎉
