# Système d'Authentification - Documentation Technique

## Vue d'ensemble

Le système d'authentification implémente un mécanisme sécurisé de gestion des utilisateurs avec hachage des mots de passe et salage (salt) pour protéger les informations sensibles.

## Architecture du Système

### 1. Structure des Données

#### Structure Utilisateur (`User`)
```c
typedef struct {
    char username[MAX_USERNAME];          // Nom d'utilisateur (max 50 caractères)
    char password_hash[HASH_SIZE * 2 + 1]; // Hash du mot de passe (128 caractères + '\0')
    char salt[SALT_SIZE * 2 + 1];         // Salt unique (32 caractères + '\0')
} User;
```

### 2. Constantes de Sécurité

- **HASH_SIZE**: 64 octets (SHA-256 produit 32 octets, stockés en 64 caractères hexadécimaux)
- **SALT_SIZE**: 16 octets (stockés en 32 caractères hexadécimaux)
- **MAX_USERS**: 100 utilisateurs maximum
- **CREDENTIALS_FILE**: "credentials.txt"

## Processus d'Authentification

### A. Inscription d'un Nouvel Utilisateur

1. **Génération du Salt**
   - Un salt aléatoire de 16 octets est généré via `/dev/urandom`
   - Converti en chaîne hexadécimale de 32 caractères
   - Le salt est unique pour chaque utilisateur

2. **Hachage du Mot de Passe**
   ```
   hash = SHA256(salt + password)
   ```
   - Le salt est concaténé avec le mot de passe
   - SHA-256 est appliqué sur cette combinaison
   - Résultat: 32 octets convertis en 64 caractères hexadécimaux

3. **Stockage des Credentials**
   - Format dans `credentials.txt`:
     ```
     username:hash:salt
     ```
   - Exemple:
     ```
     walid:a1b2c3...(64 chars):d4e5f6...(32 chars)
     ```

4. **Mise à Jour en Mémoire**
   - L'utilisateur est ajouté au tableau `users[]`
   - Le compteur `user_count` est incrémenté
   - **Important**: En mode multi-processus, chaque processus enfant doit recharger les credentials

### B. Vérification des Credentials (Login)

1. **Recherche de l'Utilisateur**
   - Recherche du nom d'utilisateur dans le tableau `users[]`
   - Si non trouvé → échec d'authentification

2. **Récupération du Salt**
   - Le salt stocké de l'utilisateur est récupéré
   - Converti de hexadécimal vers binaire

3. **Hachage du Mot de Passe Fourni**
   - Le même processus est appliqué:
     ```
     hash_tentative = SHA256(salt_stocké + password_fourni)
     ```

4. **Comparaison des Hash**
   - Le hash calculé est comparé avec le hash stocké
   - Comparaison sécurisée caractère par caractère
   - Si identiques → authentification réussie
   - Si différents → échec d'authentification

## Problème Multi-Processus et Solution

### Le Problème

Lorsque le serveur utilise `fork()` pour créer des processus enfants:

1. **Processus Parent** charge les credentials au démarrage
2. **Client A** se connecte → **Enfant 1** créé (copie de la mémoire parent)
3. **Client A** s'inscrit → **Enfant 1** met à jour sa copie locale et le fichier
4. **Enfant 1** se termine
5. **Client B** se connecte → **Enfant 2** créé (copie du parent ORIGINAL)
6. **Client B** tente de se connecter avec le nouveau compte → **ÉCHEC**
   - L'**Enfant 2** a l'ancienne copie sans le nouvel utilisateur!

### La Solution

```c
// Dans server.c, après fork()
if (client_pid == 0) {  // Processus enfant
    close(server_socket);
    
    // RECHARGER les credentials du fichier
    load_users();
    
    // Maintenant gérer le client
    handle_client(client_socket);
    exit(0);
}
```

**Principe**: Chaque processus enfant recharge les credentials depuis le fichier au démarrage, garantissant qu'il a toujours les données les plus récentes.

## Sécurité

### Points Forts

1. **Salt Unique**: Chaque utilisateur a un salt différent
   - Empêche les attaques par rainbow tables
   - Deux utilisateurs avec le même mot de passe auront des hash différents

2. **SHA-256**: Algorithme cryptographique robuste
   - 256 bits de sortie (32 octets)
   - Résistant aux collisions

3. **Pas de Mots de Passe en Clair**: 
   - Seuls les hash sont stockés
   - Impossible de récupérer le mot de passe original

4. **Protection par Mutex**:
   - `pthread_mutex_t` protège l'accès concurrent au fichier
   - Évite les conditions de course (race conditions)

### Limitations

1. **SHA-256 Simple**: Pas de KDF (Key Derivation Function)
   - Recommandation future: utiliser bcrypt, scrypt ou Argon2
   - Ces algorithmes sont intentionnellement lents pour résister au bruteforce

2. **Stockage en Fichier Texte**: 
   - Pas de chiffrement du fichier credentials.txt
   - Recommandation: permissions restrictives (chmod 600)

3. **Pas de Politique de Mot de Passe**:
   - Aucune vérification de complexité
   - Pas de longueur minimale

## Flux de Communication

```
CLIENT                                  SERVEUR
  |                                       |
  |------ REGISTER username:password ---->|
  |                                       |
  |                    [Générer salt]     |
  |                    [Hasher password]  |
  |                    [Sauvegarder]      |
  |                                       |
  |<----- "Registration successful" ------|
  |                                       |
  |------ LOGIN username:password ------->|
  |                                       |
  |                    [Charger salt]     |
  |                    [Hasher password]  |
  |                    [Comparer hash]    |
  |                                       |
  |<----- "AUTH_SUCCESS" ou "AUTH_FAIL" --|
  |                                       |
```

## Fonctions Principales

### `hash_password()`
- Entrée: password (texte), salt (binaire)
- Sortie: hash SHA-256 (binaire)
- Processus: Concatène salt+password puis applique SHA-256

### `generate_salt()`
- Génère 16 octets aléatoires depuis `/dev/urandom`
- Convertit en chaîne hexadécimale

### `create_user()`
- Crée un nouvel utilisateur
- Génère le salt et le hash
- Sauvegarde dans le fichier et en mémoire

### `verify_credentials()`
- Vérifie si username/password correspondent
- Retourne 1 si succès, 0 si échec

### `load_users()`
- Charge tous les utilisateurs depuis credentials.txt
- Appelée au démarrage du serveur et après chaque fork()

### `save_user()`
- Ajoute un utilisateur au fichier credentials.txt
- Utilise un mutex pour la synchronisation

## Recommandations Futures

1. **Implémenter bcrypt ou Argon2** pour un hachage plus sécurisé
2. **Ajouter des politiques de mot de passe** (longueur, complexité)
3. **Chiffrer le fichier credentials.txt** au repos
4. **Ajouter un système de sessions/tokens** pour éviter de retransmettre le mot de passe
5. **Implémenter une limite de tentatives** contre le bruteforce
6. **Logger les tentatives d'authentification** pour la détection d'intrusions
7. **Utiliser TLS/SSL** pour chiffrer les communications réseau

---

*Documentation créée le 12 décembre 2025*
