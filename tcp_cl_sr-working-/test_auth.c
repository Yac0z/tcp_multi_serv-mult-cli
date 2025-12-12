#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

void bytes_to_hex(const unsigned char *data, size_t len, char *hex_string) {
    for (size_t i = 0; i < len; i++) {
        sprintf(hex_string + (i * 2), "%02x", data[i]);
    }
    hex_string[len * 2] = '\0';
}

int main() {
    // Test with walid's salt and password
    unsigned char salt_bytes[16];
    char salt[] = "32bf70d81f6dd9a78cb6666949bf8721";
    char password[] = "walidd";
    
    // Convert hex salt to bytes
    for (int i = 0; i < 16; i++) {
        sscanf(salt + 2*i, "%2hhx", &salt_bytes[i]);
    }
    
    // Hash using EVP (same as hash_password function)
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, salt_bytes, 16);  // SALT first
    EVP_DigestUpdate(ctx, password, strlen(password));  // then PASSWORD
    
    unsigned char hash[32];
    unsigned int hash_len;
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);
    
    char hex_hash[65];
    bytes_to_hex(hash, 32, hex_hash);
    
    printf("Password: %s\n", password);
    printf("Salt: %s\n", salt);
    printf("Computed hash: %s\n", hex_hash);
    printf("Stored hash:   bd3a779174a5d20cb29be2a8ee20b694bce1ca9a02ad1f4acde6b7643994dee0\n");
    printf("Match: %s\n", strcmp(hex_hash, "bd3a779174a5d20cb29be2a8ee20b694bce1ca9a02ad1f4acde6b7643994dee0") == 0 ? "YES" : "NO");
    
    return 0;
}
