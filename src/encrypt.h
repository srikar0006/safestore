#ifndef SAFESTORE_ENCRYPT_H
#define SAFESTORE_ENCRYPT_H

/* Prompts (with confirmation) for a password on the controlling terminal,
 * then encrypts in_path to out_path with AES-256-CBC. */
void encrypt_file(const char *in_path, const char *out_path);

/* Prompts once for a password on the controlling terminal and decrypts
 * in_path to out_path. Exits the process if decryption fails (e.g. wrong
 * password). */
void decrypt_file(const char *in_path, const char *out_path);

#endif /* SAFESTORE_ENCRYPT_H */
