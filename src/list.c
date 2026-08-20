#include "common.h"
#include "list.h"

void do_list(const char *home) {
    char db_path[PATH_MAX];
    snprintf(db_path, sizeof(db_path), "%s" STORE "/file.csv", home);

    FILE *db = fopen(db_path, "r");
    if (!db) {
        printf("No backups yet.\n");
        return;
    }

    entry_t *entries = NULL;
    int count = 0;

    char line[PATH_MAX + 128];
    fgets(line, sizeof(line), db);
    while (fgets(line, sizeof(line), db)) {
        entries = realloc(entries, (count + 1) * sizeof(entry_t));
        if (sscanf(line, "%31[^,],\"%4095[^\"]\",%40s",
                   entries[count].timestamp, entries[count].path,
                   entries[count].hash) == 3)
            count++;
    }
    fclose(db);

    for (int i = 0; i < count; i++) {
        int already_shown = 0;
        for (int j = 0; j < i; j++) {
            if (strcmp(entries[j].path, entries[i].path) == 0) { already_shown = 1; break; }
        }
        if (already_shown) continue;

        printf("%s\n", entries[i].path);
        for (int j = i; j < count; j++) {
            if (strcmp(entries[j].path, entries[i].path) == 0)
                printf("  %s  %s\n", entries[j].timestamp, entries[j].hash);
        }
    }

    free(entries);
}
