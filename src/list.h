#ifndef SAFESTORE_LIST_H
#define SAFESTORE_LIST_H

#include <limits.h>

typedef struct {
    char timestamp[32];
    char path[PATH_MAX];
    char hash[41];
} entry_t;

/* Prints all backed up paths grouped with their timestamp/hash history. */
void do_list(const char *home);

#endif /* SAFESTORE_LIST_H */
