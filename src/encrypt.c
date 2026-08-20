#include "common.h"
#include "encrypt.h"
#include <termios.h>

static void read_password(const char *prompt, char *out, size_t outsz) {
    printf("%s", prompt);
    fflush(stdout);

    struct termios oldt, newt;
    int have_tty = tcgetattr(STDIN_FILENO, &oldt) == 0;
    if (have_tty) {
        newt = oldt;
        newt.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }

    if (!fgets(out, outsz, stdin)) out[0] = '\0';

    if (have_tty) tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");

    out[strcspn(out, "\n")] = '\0';
}

/* Runs `openssl enc ...` and feeds pass followed by a newline to its
 * stdin (openssl's -pass stdin reads a single line as the passphrase). */
static int run_openssl(const char *args_fmt, const char *pass,
                        const char *in_path, const char *out_path) {
    char cmd[PATH_MAX * 2 + 256];
    snprintf(cmd, sizeof(cmd), args_fmt, in_path, out_path);

    FILE *p = popen(cmd, "w");
    if (!p) return -1;
    fprintf(p, "%s\n", pass);
    int status = pclose(p);
    return status;
}

void encrypt_file(const char *in_path, const char *out_path) {
    char pass1[256], pass2[256];

    for (;;) {
        read_password("Enter password for backup encryption: ", pass1, sizeof(pass1));
        read_password("Confirm password: ", pass2, sizeof(pass2));
        if (pass1[0] != '\0' && strcmp(pass1, pass2) == 0) break;
        printf("Passwords didn't match, try again.\n");
    }

    int status = run_openssl(
        "openssl enc -aes-256-cbc -pbkdf2 -salt -pass stdin -in '%s' -out '%s'",
        pass1, in_path, out_path);

    memset(pass1, 0, sizeof(pass1));
    memset(pass2, 0, sizeof(pass2));

    if (status != 0) {
        printf("Encryption failed\n");
        remove(out_path);
        exit(1);
    }
}

void decrypt_file(const char *in_path, const char *out_path) {
    char pass[256];
    read_password("Enter password to decrypt backup: ", pass, sizeof(pass));

    int status = run_openssl(
        "openssl enc -d -aes-256-cbc -pbkdf2 -salt -pass stdin -in '%s' -out '%s'",
        pass, in_path, out_path);

    memset(pass, 0, sizeof(pass));

    if (status != 0) {
        printf("Decryption failed (wrong password?)\n");
        remove(out_path);
        exit(1);
    }
}
