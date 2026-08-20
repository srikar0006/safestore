#include "common.h"
#include "backup.h"
#include "hash.h"
#include "fsutil.h"
#include "compress.h"
#include "encrypt.h"

void do_backup(const char *home, const char *given_path, int compress, int encrypt) {
    char abspath[PATH_MAX];
    if (!realpath(given_path, abspath)) {
        printf("Couldn't find '%s'\n", given_path);
        exit(1);
    }

    char hash[41];
    sha1_hex(abspath, hash);

    char folder_stamp[32], log_stamp[32];
    FILE *d = popen("date '+%Y%m%d-%H%M%S %Y-%m-%d %H:%M:%S'", "r");
    fscanf(d, "%31s %31[^\n]", folder_stamp, log_stamp);
    pclose(d);

    char backup_dir[PATH_MAX];
    snprintf(backup_dir, sizeof(backup_dir), "%s" STORE "/Backup/%s/%s",
             home, hash, folder_stamp);
    mkdir_p(backup_dir);

    const char *base = strrchr(abspath, '/');
    base = base ? base + 1 : abspath;

    char dest[PATH_MAX];

    if (!compress && !encrypt) {
        snprintf(dest, sizeof(dest), "%s/%s", backup_dir, base);
        copy_path(abspath, dest);
    } else {
        const char *archive_ext = compress ? ".tar.lz4" : ".tar";

        char archive[PATH_MAX];
        snprintf(archive, sizeof(archive), "/tmp/safestore_%s_XXXXXX%s", folder_stamp, archive_ext);
        int archive_fd = mkstemps(archive, (int)strlen(archive_ext));
        if (archive_fd == -1) { printf("Couldn't create temp archive\n"); exit(1); }
        close(archive_fd);

        archive_path(abspath, archive, compress);

        if (encrypt) {
            snprintf(dest, sizeof(dest), "%s/%s%s.enc", backup_dir, base, archive_ext);
            encrypt_file(archive, dest);
            remove(archive);
        } else {
            snprintf(dest, sizeof(dest), "%s/%s%s", backup_dir, base, archive_ext);
            char mv_cmd[PATH_MAX * 2 + 16];
            snprintf(mv_cmd, sizeof(mv_cmd), "mv '%s' '%s'", archive, dest);
            system(mv_cmd);
        }
    }

    char db_path[PATH_MAX];
    snprintf(db_path, sizeof(db_path), "%s" STORE "/file.csv", home);
    int db_is_new = access(db_path, F_OK) != 0;

    FILE *db = fopen(db_path, "a");
    if (db_is_new) fprintf(db, "timestamp,path,hash\n");
    fprintf(db, "%s,\"%s\",%s\n", log_stamp, abspath, hash);
    fclose(db);

    printf("Backed up '%s' to '%s'\n", abspath, dest);
}
