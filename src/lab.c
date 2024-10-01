#include "lab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

int change_dir(char **dir);

char **cmd_parse(char const *line);

void cmd_free(char **line);

char *trim_white(char *line);

bool do_builtin(struct shell *sh, char **argv);

void sh_init(struct shell *sh);

void sh_destroy(struct shell *sh);

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