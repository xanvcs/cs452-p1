#include "lab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <readline/history.h>
#include <signal.h>
#include <pwd.h>

char *get_prompt(const char *env) {
    const char *prompt_env = getenv(env);
    char *prompt;
    size_t prompt_length;

    if (prompt_env != NULL) {
        prompt_length = strlen(prompt_env) + 1;
    } else {
        const char *default_prompt = "shell> ";
        prompt_length = strlen(default_prompt) + 1;
        prompt_env = default_prompt;
    }

    prompt = (char *)malloc(prompt_length * sizeof(char));
    if (prompt == NULL) {
        fprintf(stderr, "Error: Unable to allocate memory for prompt\n");
        return NULL;
    }

    strncpy(prompt, prompt_env, prompt_length);
    prompt[prompt_length - 1] = '\0';

    return prompt;
}

int change_dir(char **dir) {
    const char *new_dir;

    if (dir[0] == NULL) {
        new_dir = getenv("HOME");
        if (new_dir == NULL) {
            struct passwd *pw = getpwuid(getuid());
            if (pw == NULL) {
                fprintf(stderr, "Error: Unable to get home directory\n");
                return -1;
            }
            new_dir = pw->pw_dir;
        }
    } else {
        new_dir = dir[0];
    }

    if (chdir(new_dir) != 0) {
        perror("cd");
        return -1;
    }

    return 0;
}

char **cmd_parse(char const *line);

void cmd_free(char **line);

char *trim_white(char *line);

bool do_builtin(struct shell *sh, char **argv) {
    if (argv[0] == NULL) {
        return false;
    }

    if (strcmp(argv[0], "exit") == 0) {
        printf("Exiting shell...\n");
        sh_destroy(sh);
        exit(0);
    }

    if (strcmp(argv[0], "cd") == 0) {
        return change_dir(argv + 1);
    }

    if (strcmp(argv[0], "history") == 0) {
        HIST_ENTRY *entry;
        int i;

        for (i = 0; i < history_length; i++) {
            entry = history_get(i + 1);
            if (entry) {
                printf("%d: %s\n", i + 1, entry->line);
            }
        }
        return true;
    }

    return false;
}

void sh_init(struct shell *sh) {
    sh->shell_terminal = STDIN_FILENO;
    sh->shell_is_interactive = isatty(sh->shell_terminal);
    sh->prompt = NULL;

    if (sh->shell_is_interactive) {
        while (tcgetpgrp(sh->shell_terminal) != (sh->shell_pgid = getpgrp())) {
            kill(-sh->shell_pgid, SIGTTIN);
        }
        tcgetattr(sh->shell_terminal, &sh->shell_tmodes);
    }

    using_history();
}

void sh_destroy(struct shell *sh) {
    if (sh != NULL) {
        if (sh->prompt != NULL) {
            free(sh->prompt);
        }
        free(sh);
    }
}

void parse_args(int argc, char **argv) {
    int opt;

    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {
            case 'v':
                printf("v%d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
                exit(0);
            default:
                fprintf(stderr, "Usage: %s [-v]\n", argv[0]);
                exit(1);
        }
    }
}