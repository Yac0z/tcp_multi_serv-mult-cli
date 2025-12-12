#include "auth.h"

// Global storage with thread safety
static User users[MAX_USERS];
static int user_count = 0;
static Session sessions[MAX_USERS];
static int session_count = 0;
static pthread_mutex_t auth_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t session_mutex = PTHREAD_MUTEX_INITIALIZER;

#define CREDENTIALS_FILE "data/credentials.dat"

// ============================================================================
// Utility Functions
// ============================================================================

void generate_salt(unsigned char *salt, size_t size) {
    if (RAND_bytes(salt, size) != 1) {
        fprintf(stderr, "[AUTH ERROR] Failed to generate salt\n");
        exit(1);
    }
}

void bytes_to_hex(const unsigned char *bytes, size_t len, char *hex) {
    for (size_t i = 0; i < len; i++) {
        sprintf(hex + (i * 2), "%02x", bytes[i]);
    }
    hex[len * 2] = '\0';
}

void hex_to_bytes(const char *hex, unsigned char *bytes, size_t len) {
    for (size_t i = 0; i < len; i++) {
        sscanf(hex + (i * 2), "%2hhx", &bytes[i]);
    }
}

void hash_password(const char *password, const unsigned char *salt, 
                   unsigned char *hash) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        fprintf(stderr, "[AUTH ERROR] Failed to create hash context\n");
        exit(1);
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1) {
        fprintf(stderr, "[AUTH ERROR] Failed to initialize hash\n");
        EVP_MD_CTX_free(ctx);
        exit(1);
    }

    EVP_DigestUpdate(ctx, salt, SALT_SIZE);
    EVP_DigestUpdate(ctx, password, strlen(password));
    
    unsigned int hash_len;
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);
}

// ============================================================================
// Validation Functions
// ============================================================================

int validate_username(const char *username) {
    if (username == NULL || strlen(username) == 0) {
        fprintf(stderr, "[AUTH VALIDATION] Username is NULL or empty\n");
        return 0; // Empty username
    }
    
    size_t len = strlen(username);
    if (len >= MAX_USERNAME) {
        fprintf(stderr, "[AUTH VALIDATION] Username too long: %zu chars\n", len);
        return 0; // Too long
    }
    
    // Check for valid characters (alphanumeric, underscore, hyphen)
    for (size_t i = 0; i < len; i++) {
        if (!isalnum(username[i]) && username[i] != '_' && username[i] != '-') {
            fprintf(stderr, "[AUTH VALIDATION] Invalid character in username at position %zu: '%c' (0x%02x)\n", 
                    i, username[i], (unsigned char)username[i]);
            return 0; // Invalid character
        }
    }
    
    return 1; // Valid
}

int validate_password(const char *password) {
    if (password == NULL || strlen(password) < MIN_PASSWORD_LENGTH) {
        fprintf(stderr, "[AUTH VALIDATION] Password is NULL or too short: %zu chars (min: %d)\n", 
                password ? strlen(password) : 0, MIN_PASSWORD_LENGTH);
        return 0; // Too short or null
    }
    
    if (strlen(password) >= MAX_PASSWORD) {
        fprintf(stderr, "[AUTH VALIDATION] Password too long: %zu chars\n", strlen(password));
        return 0; // Too long
    }
    
    return 1; // Valid
}

// ============================================================================
// User Management Functions
// ============================================================================

int create_user(const char *username, const char *password) {
    pthread_mutex_lock(&auth_mutex);
    
    if (user_count >= MAX_USERS) {
        fprintf(stderr, "[AUTH] Maximum users reached\n");
        pthread_mutex_unlock(&auth_mutex);
        return 0;
    }

    // Check if user already exists
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            fprintf(stderr, "[AUTH] User already exists: %s\n", username);
            pthread_mutex_unlock(&auth_mutex);
            return 0;
        }
    }

    User new_user;
    strncpy(new_user.username, username, MAX_USERNAME - 1);
    new_user.username[MAX_USERNAME - 1] = '\0';

    // Generate salt
    unsigned char salt[SALT_SIZE];
    generate_salt(salt, SALT_SIZE);
    bytes_to_hex(salt, SALT_SIZE, new_user.salt);

    // Hash password
    unsigned char hash[HASH_SIZE];
    hash_password(password, salt, hash);
    bytes_to_hex(hash, SHA256_DIGEST_LENGTH, new_user.password_hash);

    users[user_count++] = new_user;
    
    // Save to file
    int result = save_user(&new_user);
    pthread_mutex_unlock(&auth_mutex);
    
    return result;
}

int register_user(const char *username, const char *password) {
    // Validate input
    if (!validate_username(username)) {
        fprintf(stderr, "[AUTH] Invalid username format\n");
        return -3; // Invalid format
    }
    
    if (!validate_password(password)) {
        fprintf(stderr, "[AUTH] Invalid password (min %d characters)\n", 
                MIN_PASSWORD_LENGTH);
        return -3; // Invalid format
    }
    
    pthread_mutex_lock(&auth_mutex);
    
    // Check if username already exists
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            pthread_mutex_unlock(&auth_mutex);
            return -1; // Username already exists
        }
    }
    
    // Check if we have space for new user
    if (user_count >= MAX_USERS) {
        pthread_mutex_unlock(&auth_mutex);
        return -2; // Too many users
    }
    
    pthread_mutex_unlock(&auth_mutex);
    
    // Create the new user
    if (create_user(username, password) == 1) {
        printf("[AUTH] New user registered: %s\n", username);
        return 0; // Success
    }
    
    return -3; // Failed to create user
}

int verify_credentials(const char *username, const char *password) {
    pthread_mutex_lock(&auth_mutex);
    
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            // Convert stored salt from hex
            unsigned char salt[SALT_SIZE];
            hex_to_bytes(users[i].salt, salt, SALT_SIZE);

            // Hash provided password
            unsigned char hash[HASH_SIZE];
            hash_password(password, salt, hash);

            // Convert to hex for comparison
            char hash_hex[HASH_SIZE * 2 + 1];
            bytes_to_hex(hash, SHA256_DIGEST_LENGTH, hash_hex);

            // Compare hashes
            int result = (strcmp(hash_hex, users[i].password_hash) == 0) ? 1 : 0;
            pthread_mutex_unlock(&auth_mutex);
            return result;
        }
    }
    
    pthread_mutex_unlock(&auth_mutex);
    return 0; // User not found
}

// ============================================================================
// Session Management Functions
// ============================================================================

void generate_token(char *token_hex, size_t size) {
    unsigned char token[TOKEN_SIZE];
    if (RAND_bytes(token, TOKEN_SIZE) != 1) {
        fprintf(stderr, "[AUTH ERROR] Failed to generate token\n");
        exit(1);
    }
    bytes_to_hex(token, TOKEN_SIZE, token_hex);
}

void cleanup_expired_sessions() {
    pthread_mutex_lock(&session_mutex);
    
    time_t now = time(NULL);
    int j = 0;
    for (int i = 0; i < session_count; i++) {
        if (sessions[i].active && sessions[i].expiry > now) {
            sessions[j++] = sessions[i];
        }
    }
    session_count = j;
    
    pthread_mutex_unlock(&session_mutex);
}

int create_session(const char *username, char *token_out) {
    cleanup_expired_sessions();
    
    pthread_mutex_lock(&session_mutex);
    
    // Check if session limit reached
    if (session_count >= MAX_USERS) {
        fprintf(stderr, "[AUTH] Maximum sessions reached\n");
        pthread_mutex_unlock(&session_mutex);
        return 0;
    }

    // Create new session
    Session new_session;
    strncpy(new_session.username, username, MAX_USERNAME - 1);
    new_session.username[MAX_USERNAME - 1] = '\0';
    
    generate_token(new_session.token, TOKEN_SIZE * 2 + 1);
    new_session.expiry = time(NULL) + SESSION_TIMEOUT;
    new_session.active = 1;

    sessions[session_count++] = new_session;
    
    strcpy(token_out, new_session.token);
    pthread_mutex_unlock(&session_mutex);
    
    return 1;
}

int verify_session(const char *username, const char *token) {
    pthread_mutex_lock(&session_mutex);
    
    time_t now = time(NULL);
    
    for (int i = 0; i < session_count; i++) {
        if (sessions[i].active &&
            strcmp(sessions[i].username, username) == 0 &&
            strcmp(sessions[i].token, token) == 0) {
            
            if (sessions[i].expiry > now) {
                // Extend session
                sessions[i].expiry = now + SESSION_TIMEOUT;
                pthread_mutex_unlock(&session_mutex);
                return 1;
            } else {
                // Session expired
                sessions[i].active = 0;
                pthread_mutex_unlock(&session_mutex);
                return 0;
            }
        }
    }
    
    pthread_mutex_unlock(&session_mutex);
    return 0;
}

int invalidate_session(const char *username, const char *token) {
    pthread_mutex_lock(&session_mutex);
    
    for (int i = 0; i < session_count; i++) {
        if (strcmp(sessions[i].username, username) == 0 &&
            strcmp(sessions[i].token, token) == 0) {
            sessions[i].active = 0;
            pthread_mutex_unlock(&session_mutex);
            return 1;
        }
    }
    
    pthread_mutex_unlock(&session_mutex);
    return 0;
}

// ============================================================================
// File I/O Functions
// ============================================================================

int load_users() {
    pthread_mutex_lock(&auth_mutex);
    
    FILE *fp = fopen(CREDENTIALS_FILE, "r");
    if (fp == NULL) {
        // File doesn't exist, create it
        fp = fopen(CREDENTIALS_FILE, "w");
        if (fp == NULL) {
            perror("[AUTH ERROR] Could not create credentials file");
            pthread_mutex_unlock(&auth_mutex);
            return 0;
        }
        fclose(fp);
        pthread_mutex_unlock(&auth_mutex);
        return 1;
    }

    user_count = 0;
    while (fscanf(fp, "%63s %128s %32s\n", 
                  users[user_count].username,
                  users[user_count].password_hash,
                  users[user_count].salt) == 3) {
        user_count++;
        if (user_count >= MAX_USERS) {
            break;
        }
    }

    fclose(fp);
    printf("[AUTH] Loaded %d users from credentials file\n", user_count);
    pthread_mutex_unlock(&auth_mutex);
    return 1;
}

int save_user(const User *user) {
    FILE *fp = fopen(CREDENTIALS_FILE, "a");
    if (fp == NULL) {
        perror("[AUTH ERROR] Could not open credentials file for writing");
        return 0;
    }

    fprintf(fp, "%s %s %s\n", user->username, user->password_hash, user->salt);
    fclose(fp);
    return 1;
}

// ============================================================================
// Initialization and Cleanup
// ============================================================================

int init_auth_system() {
    printf("[AUTH] Initializing authentication system...\n");
    
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    
    // Initialize mutexes (already initialized statically)
    
    // Load existing users
    if (!load_users()) {
        fprintf(stderr, "[AUTH ERROR] Failed to load users\n");
        return 0;
    }

    // If no users exist, create a default admin user
    if (user_count == 0) {
        printf("[AUTH] No users found. Creating default admin user...\n");
        if (create_user("admin", "admin123")) {
            printf("[AUTH] Default user created: admin/admin123\n");
        } else {
            fprintf(stderr, "[AUTH ERROR] Failed to create default user\n");
            return 0;
        }
    }

    printf("[AUTH] Authentication system ready (%d users loaded)\n", user_count);
    return 1;
}

void cleanup_auth_system() {
    printf("[AUTH] Cleaning up authentication system...\n");
    
    pthread_mutex_lock(&auth_mutex);
    user_count = 0;
    pthread_mutex_unlock(&auth_mutex);
    
    pthread_mutex_lock(&session_mutex);
    session_count = 0;
    pthread_mutex_unlock(&session_mutex);
    
    pthread_mutex_destroy(&auth_mutex);
    pthread_mutex_destroy(&session_mutex);
    
    printf("[AUTH] Cleanup complete\n");
}

int encrypt_data(const char *plaintext, const unsigned char *key,
                 unsigned char *ciphertext, int *ciphertext_len) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        return 0;
    }

    // Generate IV
    unsigned char iv[16];
    if (RAND_bytes(iv, 16) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    // Store IV at the beginning of ciphertext
    memcpy(ciphertext, iv, 16);

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    int len;
    if (EVP_EncryptUpdate(ctx, ciphertext + 16, &len, 
                          (unsigned char*)plaintext, strlen(plaintext)) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    *ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext + 16 + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    *ciphertext_len += len + 16; // Include IV size

    EVP_CIPHER_CTX_free(ctx);
    return 1;
}

int decrypt_data(const unsigned char *ciphertext, int ciphertext_len,
                 const unsigned char *key, char *plaintext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        return 0;
    }

    // Extract IV from beginning of ciphertext
    unsigned char iv[16];
    memcpy(iv, ciphertext, 16);

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }

    int len;
    if (EVP_DecryptUpdate(ctx, (unsigned char*)plaintext, &len,
                          ciphertext + 16, ciphertext_len - 16) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    int plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx, (unsigned char*)plaintext + len, &len) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    plaintext_len += len;
    plaintext[plaintext_len] = '\0';

    EVP_CIPHER_CTX_free(ctx);
    return 1;
}
