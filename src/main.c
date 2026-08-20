#include "common.h"
#include "backup.h"
#include "list.h"
#include "restore.h"

int main(int argc, char *argv[]) {
    char *home = getenv("HOME");

    if (argc >= 3 && (strcmp(argv[1], "-b") == 0 || strcmp(argv[1], "--backup") == 0)) {
        const char *path = NULL;
        int compress = 0, encrypt = 0;
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--compress") == 0) {
                compress = 1;
            } else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--encrypt") == 0) {
                encrypt = 1;
            } else if (!path) {
                path = argv[i];
            }
        }
        if (!path) {
            printf("Usage: %s -b|--backup <file_or_folder> [-c|--compress] [-e|--encrypt]\n", argv[0]);
            return 1;
        }
        do_backup(home, path, compress, encrypt);
        return 0;
    }

    if (argc == 2 && (strcmp(argv[1], "-l") == 0 || strcmp(argv[1], "--list") == 0)) {
        do_list(home);
        return 0;
    }

    if (argc == 3 && (strcmp(argv[1], "-r") == 0 || strcmp(argv[1], "--restore") == 0)) {
        do_restore(home, argv[2]);
        return 0;
    }

    printf("Usage: %s -b|--backup <file_or_folder> [-c|--compress] [-e|--encrypt]\n", argv[0]);
    printf("       %s -l|--list\n", argv[0]);
    printf("       %s -r|--restore <file_name>\n", argv[0]);
    return 1;
}
