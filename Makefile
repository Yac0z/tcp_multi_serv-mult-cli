# Makefile for TCP Multi-Server Multi-Client Application
# Author: TCP Server-Client Project
# Date: December 7, 2025

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = 

# Target executables
SERVER = server
CLIENT = client

# Source files
SERVER_SRC = server.c
CLIENT_SRC = client.c

# Header files (dependencies)
SERVER_HEADERS = serverdef.h serverimp.c service.c service.h
CLIENT_HEADERS = clientdef.h

# Object files (not used directly since we include .c files, but listed for clarity)
# SERVER_OBJ = server.o
# CLIENT_OBJ = client.o

# Default target - build both server and client
all: $(SERVER) $(CLIENT)
	@echo "========================================="
	@echo "Build completed successfully!"
	@echo "========================================="
	@echo "Server executable: ./$(SERVER)"
	@echo "Client executable: ./$(CLIENT)"
	@echo ""
	@echo "To run server: ./$(SERVER) <port>"
	@echo "To run client: ./$(CLIENT) <hostname> <port>"
	@echo "========================================="

# Build server
$(SERVER): $(SERVER_SRC) $(SERVER_HEADERS)
	@echo "Compiling server..."
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_SRC) $(LDFLAGS)
	@echo "Server compiled successfully!"

# Build client
$(CLIENT): $(CLIENT_SRC) $(CLIENT_HEADERS)
	@echo "Compiling client..."
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT_SRC) $(LDFLAGS)
	@echo "Client compiled successfully!"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(SERVER) $(CLIENT)
	rm -f *.o
	rm -f *~
	rm -f core
	@echo "Clean completed!"

# Rebuild everything from scratch
rebuild: clean all

# Run server in multi-process mode (for testing)
run-server-multi:
	@echo "Starting server in MULTI-PROCESS mode on port 8080..."
	@echo "1" | ./$(SERVER) 8080

# Run server in FIFO mode (for testing)
run-server-fifo:
	@echo "Starting server in FIFO mode on port 8080..."
	@echo "2" | ./$(SERVER) 8080

# Run client (connects to localhost:8080)
run-client:
	@echo "Starting client (connecting to localhost:8080)..."
	./$(CLIENT) localhost 8080

# Test - compile and run both (server in background, client in foreground)
test: all
	@echo "========================================="
	@echo "Running test scenario..."
	@echo "========================================="
	@echo "Starting server in FIFO mode..."
	@(echo "2" | ./$(SERVER) 8080 &); sleep 2
	@echo "Starting client..."
	@(echo "1"; sleep 1; echo "2"; sleep 1; echo "5") | ./$(CLIENT) localhost 8080
	@echo "Test completed!"
	@pkill -9 server 2>/dev/null || true

# Check if executables exist
check:
	@echo "Checking build status..."
	@if [ -f $(SERVER) ]; then \
		echo "✓ Server executable exists"; \
	else \
		echo "✗ Server executable missing"; \
	fi
	@if [ -f $(CLIENT) ]; then \
		echo "✓ Client executable exists"; \
	else \
		echo "✗ Client executable missing"; \
	fi

# Display help
help:
	@echo "========================================="
	@echo "TCP Server-Client Makefile Help"
	@echo "========================================="
	@echo "Available targets:"
	@echo ""
	@echo "  make              - Build both server and client (default)"
	@echo "  make all          - Same as 'make'"
	@echo "  make server       - Build only the server"
	@echo "  make client       - Build only the client"
	@echo "  make clean        - Remove all build artifacts"
	@echo "  make rebuild      - Clean and rebuild everything"
	@echo "  make run-server-multi - Run server in multi-process mode"
	@echo "  make run-server-fifo  - Run server in FIFO mode"
	@echo "  make run-client   - Run client (connects to localhost:8080)"
	@echo "  make test         - Compile and run automated test"
	@echo "  make check        - Check if executables exist"
	@echo "  make help         - Display this help message"
	@echo ""
	@echo "========================================="
	@echo "Examples:"
	@echo "========================================="
	@echo "  make && ./server 8080"
	@echo "  make && ./client localhost 8080"
	@echo "  make clean && make"
	@echo "========================================="

# Phony targets (not actual files)
.PHONY: all clean rebuild run-server-multi run-server-fifo run-client test check help
