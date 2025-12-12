#ifndef AUTH_H
#define AUTH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <time.h>
#include <pthread.h>
#include <ctype.h>

#define MAX_USERNAME 64
#define MAX_PASSWORD 128
#define MIN_PASSWORD_LENGTH 6
#define SALT_SIZE 16
#define HASH_SIZE 64
#define TOKEN_SIZE 32
#define MAX_USERS 100
#define SESSION_TIMEOUT 3600 // 1 hour in seconds

// User structure
typedef struct {
    char username[MAX_USERNAME];
    char password_hash[HASH_SIZE * 2 + 1]; // Hex string
    char salt[SALT_SIZE * 2 + 1]; // Hex string
} User;

// Session token structure
typedef struct {
    char username[MAX_USERNAME];
    char token[TOKEN_SIZE * 2 + 1]; // Hex string
    time_t expiry;
    int active;
} Session;

// Function declarations

/**
 * Initialize authentication system
 */
int init_auth_system();

/**
 * Cleanup authentication system
 */
void cleanup_auth_system();

/**
 * Generate a random salt
 */
void generate_salt(unsigned char *salt, size_t size);

/**
 * Convert binary data to hex string
 */
void bytes_to_hex(const unsigned char *bytes, size_t len, char *hex);

/**
 * Convert hex string to binary data
 */
void hex_to_bytes(const char *hex, unsigned char *bytes, size_t len);

/**
 * Hash password with salt using SHA-256
 */
void hash_password(const char *password, const unsigned char *salt, 
                   unsigned char *hash);

/**
 * Validate username format
 */
int validate_username(const char *username);

/**
 * Validate password strength
 */
int validate_password(const char *password);

/**
 * Create a new user with hashed password
 */
int create_user(const char *username, const char *password);

/**
 * Register a new user
 * Returns: 0 on success, -1 if user exists, -2 if max users, -3 on error
 */
int register_user(const char *username, const char *password);

/**
 * Verify user credentials
 * Returns: 1 if valid, 0 if invalid
 */
int verify_credentials(const char *username, const char *password);

/**
 * Generate authentication token
 */
void generate_token(char *token_hex, size_t size);

/**
 * Create session for authenticated user
 * Returns: 1 on success, 0 on failure
 */
int create_session(const char *username, char *token_out);

/**
 * Verify session token
 * Returns: 1 if valid, 0 if invalid or expired
 */
int verify_session(const char *username, const char *token);

/**
 * Invalidate a session
 * Returns: 1 on success, 0 on failure
 */
int invalidate_session(const char *username, const char *token);

/**
 * Load users from file
 */
int load_users();

/**
 * Save user to file
 */
int save_user(const User *user);

/**
 * Clean up expired sessions
 */
void cleanup_expired_sessions();

/**
 * Encrypt data using AES-256-CBC
 */
int encrypt_data(const char *plaintext, const unsigned char *key, 
                 unsigned char *ciphertext, int *ciphertext_len);

/**
 * Decrypt data using AES-256-CBC
 */
int decrypt_data(const unsigned char *ciphertext, int ciphertext_len,
                 const unsigned char *key, char *plaintext);

#endif // AUTH_H
