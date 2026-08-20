#include "common.h"
#include "compress.h"
#include "fsutil.h"

void archive_path(const char *src, const char *dst, int lz4) {
    char src_copy[PATH_MAX];
    snprintf(src_copy, sizeof(src_copy), "%s", src);

    char parent_dir[PATH_MAX], base_name[PATH_MAX];
    char *slash = strrchr(src_copy, '/');
    if (slash && slash != src_copy) {
        *slash = '\0';
        snprintf(parent_dir, sizeof(parent_dir), "%s", src_copy);
        snprintf(base_name, sizeof(base_name), "%s", slash + 1);
    } else if (slash) {
        /* src is at filesystem root, e.g. "/foo" */
        snprintf(parent_dir, sizeof(parent_dir), "/");
        snprintf(base_name, sizeof(base_name), "%s", slash + 1);
    } else {
        snprintf(parent_dir, sizeof(parent_dir), ".");
        snprintf(base_name, sizeof(base_name), "%s", src_copy);
    }

    char cmd[PATH_MAX * 3 + 128];
    if (lz4) {
        snprintf(cmd, sizeof(cmd),
                 "tar -C '%s' -cf - '%s' | lz4 -q -z > '%s'",
                 parent_dir, base_name, dst);
    } else {
        snprintf(cmd, sizeof(cmd),
                 "tar -C '%s' -cf '%s' '%s'",
                 parent_dir, dst, base_name);
    }
    system(cmd);
}

void extract_archive(const char *src, const char *dst_dir, int lz4) {
    mkdir_p(dst_dir);

    char cmd[PATH_MAX * 2 + 64];
    if (lz4) {
        snprintf(cmd, sizeof(cmd),
                 "lz4 -q -d -c '%s' | tar -C '%s' -xf -",
                 src, dst_dir);
    } else {
        snprintf(cmd, sizeof(cmd),
                 "tar -C '%s' -xf '%s'",
                 dst_dir, src);
    }
    system(cmd);
}
