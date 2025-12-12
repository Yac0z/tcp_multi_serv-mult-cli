# Makefile for TCP Multi-Server Multi-Client Application
# Author: TCP Server-Client Project
# Date: December 7, 2025

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lssl -lcrypto -lpthread 

# Target executables
SERVER = server
CLIENT = client
GUI_CLIENT = gui_client

# Source files
SERVER_SRC = server.c auth.c service.c
CLIENT_SRC = client.c
GUI_CLIENT_SRC = gui_client.c

# Header files (dependencies)
SERVER_HEADERS = serverdef.h serverimp.c service.h auth.h
CLIENT_HEADERS = clientdef.h
GUI_CLIENT_HEADERS = gui_client.h

# Object files
SERVER_OBJ = server.o auth.o service.o
CLIENT_OBJ = client.o
GUI_CLIENT_OBJ = gui_client.o

# GTK flags
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LDFLAGS = $(shell pkg-config --libs gtk+-3.0)

# Default target - build server, client, and GUI client
all: $(SERVER) $(CLIENT) $(GUI_CLIENT)
	@echo "========================================="
	@echo "Build completed successfully!"
	@echo "========================================="
	@echo "Server executable: ./$(SERVER)"
	@echo "Client executable: ./$(CLIENT)"
	@echo "GUI Client executable: ./$(GUI_CLIENT)"
	@echo ""
	@echo "To run server: ./$(SERVER) <port>"
	@echo "To run client: ./$(CLIENT) <hostname> <port>"
	@echo "To run GUI client: ./$(GUI_CLIENT)"
	@echo "========================================="

# Build server
$(SERVER): $(SERVER_OBJ)
	@echo "Linking server..."
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_OBJ) $(LDFLAGS)
	@echo "Server compiled successfully!"

# Build server object files
server.o: server.c $(SERVER_HEADERS)
	@echo "Compiling server.c..."
	$(CC) $(CFLAGS) -c server.c

auth.o: auth.c auth.h
	@echo "Compiling auth.c..."
	$(CC) $(CFLAGS) -c auth.c

service.o: service.c service.h
	@echo "Compiling service.c..."
	$(CC) $(CFLAGS) -c service.c

# Build client
$(CLIENT): $(CLIENT_OBJ)
	@echo "Linking client..."
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT_OBJ) $(LDFLAGS)
	@echo "Client compiled successfully!"

client.o: client.c $(CLIENT_HEADERS)
	@echo "Compiling client.c..."
	$(CC) $(CFLAGS) -c client.c

# Build GUI client
$(GUI_CLIENT): $(GUI_CLIENT_OBJ)
	@echo "Linking GUI client..."
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -o $(GUI_CLIENT) $(GUI_CLIENT_OBJ) $(GTK_LDFLAGS)
	@echo "GUI Client compiled successfully!"

gui_client.o: gui_client.c $(GUI_CLIENT_HEADERS)
	@echo "Compiling gui_client.c..."
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c gui_client.c

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(SERVER) $(CLIENT) $(GUI_CLIENT) $(SERVER_OBJ) $(CLIENT_OBJ) $(GUI_CLIENT_OBJ)
	@echo "Clean complete!"

# Clean everything including generated data
distclean: clean
	@echo "Cleaning all generated files..."
	rm -f data/credentials.dat
	@echo "Deep clean complete!"

# Run server in MULTI-PROCESS mode
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

# Run GUI client
run-gui:
	@echo "Starting GUI client..."
	./$(GUI_CLIENT)

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
	@if [ -f $(GUI_CLIENT) ]; then \
		echo "✓ GUI Client executable exists"; \
	else \
		echo "✗ GUI Client executable missing"; \
	fi

# Help target
help:
	@echo "========================================="
	@echo "TCP Client-Server Build System"
	@echo "========================================="
	@echo "Available targets:"
	@echo "  make                   - Build all (server, client, GUI client)"
	@echo "  make server            - Build server only"
	@echo "  make client            - Build CLI client only"
	@echo "  make gui_client        - Build GUI client only"
	@echo "  make clean             - Remove build artifacts"
	@echo "  make distclean         - Remove all generated files"
	@echo "  make run-server-multi  - Run server in multi-process mode"
	@echo "  make run-server-fifo   - Run server in FIFO mode"
	@echo "  make run-client        - Run CLI client"
	@echo "  make run-gui           - Run GUI client"
	@echo "  make test              - Run automated test"
	@echo "  make check             - Check build status"
	@echo "  make help              - Show this help"
	@echo "========================================="

.PHONY: all clean distclean run-server-multi run-server-fifo run-client run-gui test check help
