#ifndef SAFESTORE_FSUTIL_H
#define SAFESTORE_FSUTIL_H

/* Recursively creates all directories in path (like `mkdir -p`). */
void mkdir_p(const char *path);

/* Recursively copies src to dst (like `cp -r`). */
void copy_path(const char *src, const char *dst);

#endif /* SAFESTORE_FSUTIL_H */
