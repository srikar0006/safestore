#ifndef SAFESTORE_HASH_H
#define SAFESTORE_HASH_H

/* Writes the 40-char hex SHA1 digest of str into out (must hold >= 41 bytes). */
void sha1_hex(const char *str, char *out);

#endif /* SAFESTORE_HASH_H */
