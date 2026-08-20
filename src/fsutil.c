#include "common.h"
#include "fsutil.h"

void mkdir_p(const char *path) {
    char tmp[PATH_MAX];
    snprintf(tmp, sizeof(tmp), "%s", path);

    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(tmp, 0755);
            *p = '/';
        }
    }
    mkdir(tmp, 0755);
}

void copy_path(const char *src, const char *dst) {
    char cmd[PATH_MAX * 2 + 32];
    snprintf(cmd, sizeof(cmd), "cp -r '%s' '%s'", src, dst);
    system(cmd);
}
