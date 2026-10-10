
#define _POSIX_C_SOURCE 200809L

#include "pkg.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int starts_with(const char *text, const char *prefix)
{
    return strncmp(text, prefix, strlen(prefix)) == 0;
}

void show_help(void)
{
    printf(
        "pkg %s - APT package manager wrapper\n\n"
        "Usage: pkg <command> [arguments...]\n\n"
        "Commands:\n"
        "  update                Update package lists\n"
        "  upgrade               Upgrade installed packages\n"
        "  install <packages>    Install packages\n"
        "  remove <packages>     Remove packages\n"
        "  reinstall <packages>  Reinstall packages\n"
        "  search <query>        Search for packages\n"
        "  show <packages>       Show package information\n"
        "  files <packages>      List files installed by packages\n"
        "  clean                 Clear the package cache\n"
        "  autoclean             Remove obsolete cache files\n"
        "  autoremove            Remove unused packages\n"
        "  list-all              List available packages\n"
        "  list-installed        List installed packages\n"
        "  help                  Display this help message\n"
        "  --version             Display version information\n",
        PKG_VERSION
    );
}

/*
 * Run a command in a child process.
 * Arguments are passed directly to execvp(); no shell is involved.
 */
static int run_command(char *const args[])
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("pkg: fork failed");
        return 1;
    }

    if (pid == 0) {
        execvp(args[0], args);

        fprintf(stderr, "pkg: cannot execute %s: %s\n",
                args[0], strerror(errno));
        _exit(127);
    }

    int status;

    for (;;) {
        if (waitpid(pid, &status, 0) >= 0)
            break;

        if (errno != EINTR) {
            perror("pkg: waitpid failed");
            return 1;
        }
    }

    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    if (WIFSIGNALED(status))
        return 128 + WTERMSIG(status);

    return 1;
}

/*
 * Termux-compatible command matching.
 *
 * These shortcuts are intentionally not displayed by show_help().
 * Matching order follows the APT command case patterns in Termux's
 * official scripts/pkg.in.
 */
static const char *resolve_command(const char *cmd)
{
    if (strcmp(cmd, "--version") == 0)
        return "version";

    if (strcmp(cmd, "--help") == 0 ||
        strcmp(cmd, "-h") == 0 ||
        strcmp(cmd, "help") == 0 ||
        starts_with(cmd, "h"))
        return "help";

    if (starts_with(cmd, "f"))
        return "files";

    if (starts_with(cmd, "sh") ||
        starts_with(cmd, "inf"))
        return "show";

    if (strcmp(cmd, "add") == 0 ||
        starts_with(cmd, "i"))
        return "install";

    if (starts_with(cmd, "autoc"))
        return "autoclean";

    if (starts_with(cmd, "cl"))
        return "clean";

    if (starts_with(cmd, "list-a"))
        return "list-all";

    if (starts_with(cmd, "list-i"))
        return "list-installed";

    if (starts_with(cmd, "rei"))
        return "reinstall";

    if (starts_with(cmd, "se"))
        return "search";

    if (starts_with(cmd, "un") ||
        starts_with(cmd, "rem") ||
        strcmp(cmd, "rm") == 0 ||
        starts_with(cmd, "del"))
        return "remove";

    if (starts_with(cmd, "upd"))
        return "update";

    if (strcmp(cmd, "up") == 0 ||
        starts_with(cmd, "upg"))
        return "upgrade";

    /*
     * Additional commands from the original Ubuntu wrapper.
     */
    if (strcmp(cmd, "autoremove") == 0)
        return "autoremove";

    return NULL;
}

static int needs_argument(const char *cmd)
{
    return strcmp(cmd, "install") == 0 ||
           strcmp(cmd, "remove") == 0 ||
           strcmp(cmd, "reinstall") == 0 ||
           strcmp(cmd, "search") == 0 ||
           strcmp(cmd, "show") == 0 ||
           strcmp(cmd, "files") == 0;
}

int run_pkg(int argc, char *argv[])
{
    if (argc < 2) {
        show_help();
        return 0;
    }

    const char *cmd = resolve_command(argv[1]);

    if (cmd == NULL) {
        fprintf(stderr, "pkg: unknown command: %s\n", argv[1]);
        fprintf(stderr, "Run 'pkg help' for usage information.\n");
        return 2;
    }

    if (strcmp(cmd, "help") == 0) {
        show_help();
        return 0;
    }

    if (strcmp(cmd, "version") == 0) {
        printf("pkg %s\n", PKG_VERSION);
        return 0;
    }

    if (needs_argument(cmd) && argc < 3) {
        fprintf(stderr,
                "pkg: command requires an argument: %s\n",
                argv[1]);
        return 2;
    }

    /*
     * Allocate enough slots for the command, flags, forwarded
     * arguments and the terminating NULL pointer.
     */
    char **args = calloc((size_t)argc + 6, sizeof(*args));

    if (args == NULL) {
        perror("pkg: memory allocation failed");
        return 1;
    }

    int n = 0;

    if (strcmp(cmd, "upgrade") == 0) {
        char *update_args[] = {
            "apt", "update", NULL
        };

        int result = run_command(update_args);

        if (result != 0) {
            free(args);
            return result;
        }

        args[n++] = "apt";
        args[n++] = "full-upgrade";
        args[n++] = "-y";
    } else if (strcmp(cmd, "files") == 0) {
        args[n++] = "dpkg";
        args[n++] = "-L";
    } else if (strcmp(cmd, "list-installed") == 0) {
        args[n++] = "apt";
        args[n++] = "list";
        args[n++] = "--installed";
    } else {
        args[n++] = "apt";

        if (strcmp(cmd, "update") == 0) {
            args[n++] = "update";
        } else if (strcmp(cmd, "install") == 0) {
            args[n++] = "install";
            args[n++] = "-y";
        } else if (strcmp(cmd, "remove") == 0) {
            args[n++] = "remove";
            args[n++] = "-y";
        } else if (strcmp(cmd, "reinstall") == 0) {
            args[n++] = "install";
            args[n++] = "--reinstall";
        } else if (strcmp(cmd, "search") == 0) {
            args[n++] = "search";
        } else if (strcmp(cmd, "show") == 0) {
            args[n++] = "show";
        } else if (strcmp(cmd, "list-all") == 0) {
            args[n++] = "list";
        } else if (strcmp(cmd, "autoclean") == 0) {
            args[n++] = "autoclean";
        } else if (strcmp(cmd, "clean") == 0) {
            args[n++] = "clean";
        } else if (strcmp(cmd, "autoremove") == 0) {
            args[n++] = "autoremove";
            args[n++] = "-y";
        }
    }

    /*
     * Forward user arguments without invoking a shell.
     */
    for (int i = 2; i < argc; i++)
        args[n++] = argv[i];

    args[n] = NULL;

    int result = run_command(args);

    free(args);
    return result;
}

int main(int argc, char *argv[])
{
    return run_pkg(argc, argv);
}
