#ifndef SAFESTORE_COMPRESS_H
#define SAFESTORE_COMPRESS_H

/* Archives src (a file or directory, given as an absolute path) into a
 * single tar file written to dst. If lz4 is non-zero the tar stream is
 * also lz4-compressed (dst should then be named *.tar.lz4, otherwise
 * *.tar). The archive preserves only the basename of src, so extracting
 * it into a directory reproduces just that file/folder (not its full
 * original path). */
void archive_path(const char *src, const char *dst, int lz4);

/* Extracts a tar archive at src (lz4-compressed if lz4 is non-zero) into
 * the directory dst_dir (created if missing), reproducing whatever
 * basename was stored by archive_path(). */
void extract_archive(const char *src, const char *dst_dir, int lz4);

#endif /* SAFESTORE_COMPRESS_H */
