#include "common.h"
#include "hash.h"

void sha1_hex(const char *str, char *out) {
    char cmd[PATH_MAX + 64];
    snprintf(cmd, sizeof(cmd), "printf '%%s' '%s' | sha1sum", str);
    FILE *p = popen(cmd, "r");
    fscanf(p, "%40s", out);
    pclose(p);
}
