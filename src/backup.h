#ifndef SAFESTORE_BACKUP_H
#define SAFESTORE_BACKUP_H

/* Backs up given_path (file or directory) into the store under $HOME,
 * logging an entry to the CSV database. If compress is non-zero the
 * backup is archived as tar.lz4 (plain tar if encrypt is set but
 * compress isn't); otherwise it is copied as-is. If encrypt is non-zero
 * the resulting archive is additionally encrypted (prompting for a
 * password). Exits the process on error. */
void do_backup(const char *home, const char *given_path, int compress, int encrypt);

#endif /* SAFESTORE_BACKUP_H */
