#include "common.h"
#include "restore.h"
#include "hash.h"
#include "fsutil.h"
#include "compress.h"
#include "encrypt.h"
#include <dirent.h>

/* folder_stamp entries look like "20240131-235959" (8 digits, '-', 6 digits). */
static int is_stamp(const char *name) {
    if (strlen(name) != 15 || name[8] != '-') return 0;
    for (int i = 0; i < 15; i++) {
        if (i == 8) continue;
        if (name[i] < '0' || name[i] > '9') return 0;
    }
    return 1;
}

static int cmp_desc(const void *a, const void *b) {
    return strcmp((const char *)b, (const char *)a);
}

/* Resolves given_path to an absolute path, even if it no longer exists
 * (e.g. it was deleted and we're restoring it), matching the abspath that
 * do_backup() would have produced when it was still present. */
static void resolve_abspath(const char *given_path, char *abspath) {
    if (realpath(given_path, abspath)) return;

    if (given_path[0] == '/') {
        snprintf(abspath, PATH_MAX, "%s", given_path);
        return;
    }

    char cwd[PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd))) {
        printf("Couldn't resolve current directory\n");
        exit(1);
    }
    snprintf(abspath, PATH_MAX, "%s/%s", cwd, given_path);
}

void do_restore(const char *home, const char *given_path) {
    char abspath[PATH_MAX];
    resolve_abspath(given_path, abspath);

    char hash[41];
    sha1_hex(abspath, hash);

    char backup_root[PATH_MAX];
    snprintf(backup_root, sizeof(backup_root), "%s" STORE "/Backup/%s", home, hash);

    DIR *dir = opendir(backup_root);
    if (!dir) {
        printf("No backups found for '%s'\n", abspath);
        exit(1);
    }

    char stamps[1024][32];
    int count = 0;
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL && count < 1024) {
        if (is_stamp(ent->d_name)) {
            snprintf(stamps[count], sizeof(stamps[count]), "%s", ent->d_name);
            count++;
        }
    }
    closedir(dir);

    if (count == 0) {
        printf("No backups found for '%s'\n", abspath);
        exit(1);
    }

    qsort(stamps, count, sizeof(stamps[0]), cmp_desc);

    const char *base = strrchr(abspath, '/');
    base = base ? base + 1 : abspath;

    char list_path[] = "/tmp/safestore_list_XXXXXX";
    int list_fd = mkstemp(list_path);
    if (list_fd == -1) { printf("Couldn't create temp file\n"); exit(1); }
    FILE *list_f = fdopen(list_fd, "w");
    for (int i = 0; i < count; i++) {
        const char *s = stamps[i];
        fprintf(list_f, "%s\t%.4s-%.2s-%.2s %.2s:%.2s:%.2s\n",
                s, s, s + 4, s + 6, s + 9, s + 11, s + 13);
    }
    fclose(list_f);

    char cmd[PATH_MAX * 2 + 256];
    snprintf(cmd, sizeof(cmd),
             "fzf --delimiter='\t' --with-nth=2 "
             "--header='Select a backup of %s to restore' "
             "--height 40%% --layout=reverse "
             "< '%s'",
             base, list_path);

    FILE *p = popen(cmd, "r");
    char selection[64] = {0};
    if (p) {
        if (!fgets(selection, sizeof(selection), p)) selection[0] = '\0';
        pclose(p);
    }

    remove(list_path);

    char *tab = strchr(selection, '\t');
    if (tab) *tab = '\0';
    else selection[strcspn(selection, "\n")] = '\0';

    if (selection[0] == '\0') {
        printf("Restore cancelled.\n");
        return;
    }

    /* Try each possible backup form, most-specific first: compressed +
     * encrypted, encrypted-only, compressed-only, plain archive, and
     * finally a plain uncompressed/unencrypted copy. */
    struct { const char *suffix; int compress; int encrypt; } forms[] = {
        { ".tar.lz4.enc", 1, 1 },
        { ".tar.enc",     0, 1 },
        { ".tar.lz4",     1, 0 },
        { ".tar",         0, 0 },
    };

    char snapshot_dir[PATH_MAX];
    snprintf(snapshot_dir, sizeof(snapshot_dir), "%s/%s", backup_root, selection);

    char src[PATH_MAX];
    int found = 0, compress = 0, encrypt = 0;
    for (size_t i = 0; i < sizeof(forms) / sizeof(forms[0]); i++) {
        snprintf(src, sizeof(src), "%s/%s%s", snapshot_dir, base, forms[i].suffix);
        if (access(src, F_OK) == 0) {
            compress = forms[i].compress;
            encrypt = forms[i].encrypt;
            found = 1;
            break;
        }
    }
    if (!found) {
        snprintf(src, sizeof(src), "%s/%s", snapshot_dir, base);
        if (access(src, F_OK) != 0) {
            printf("Backup snapshot '%s' is missing its data\n", selection);
            exit(1);
        }
    }

    char parent_dir[PATH_MAX];
    snprintf(parent_dir, sizeof(parent_dir), "%s", abspath);
    char *slash = strrchr(parent_dir, '/');
    if (slash && slash != parent_dir) *slash = '\0';
    else if (slash) parent_dir[1] = '\0';

    char rm_cmd[PATH_MAX + 16];
    snprintf(rm_cmd, sizeof(rm_cmd), "rm -rf '%s'", abspath);
    system(rm_cmd);

    if (!found) {
        copy_path(src, abspath);
    } else if (encrypt) {
        const char *archive_ext = compress ? ".tar.lz4" : ".tar";
        char archive[PATH_MAX];
        snprintf(archive, sizeof(archive), "/tmp/safestore_%s_XXXXXX%s", selection, archive_ext);
        int archive_fd = mkstemps(archive, (int)strlen(archive_ext));
        if (archive_fd == -1) { printf("Couldn't create temp archive\n"); exit(1); }
        close(archive_fd);

        decrypt_file(src, archive);
        extract_archive(archive, parent_dir, compress);
        remove(archive);
    } else {
        extract_archive(src, parent_dir, compress);
    }

    printf("Restored '%s' from backup '%s' to '%s'\n", base, selection, abspath);
}
