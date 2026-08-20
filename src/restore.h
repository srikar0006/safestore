#ifndef SAFESTORE_RESTORE_H
#define SAFESTORE_RESTORE_H

/* Lets the user pick a previous backup of given_path (via fzf, with a
 * preview of each snapshot) and restores the chosen snapshot back to its
 * original location. Exits the process on unrecoverable error. */
void do_restore(const char *home, const char *given_path);

#endif /* SAFESTORE_RESTORE_H */
