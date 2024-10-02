#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../src/lab.h"

void check_background_processes(void);

int main(int argc, char **argv) {
    parse_args(argc, argv);

    struct shell *sh = malloc(sizeof(struct shell));
    if (sh == NULL) {
        fprintf(stderr, "Failed to allocate memory for shell\n");
        return 1;
    }

    sh_init(sh);

    char *prompt = get_prompt("MY_PROMPT");
    if (prompt == NULL) {
        fprintf(stderr, "Failed to get prompt\n");
        sh_destroy(sh);
        return 1;
    }

    using_history();
    char *line;

    while ((line = readline(prompt))) {
        check_background_processes();
        remove_completed_jobs();
        trim_white(line);
        if (strlen(line) > 0) {
            add_history(line);
            char **args = cmd_parse(line);
            if (args != NULL) {
                do_builtin(sh, args);
                cmd_free(args);
            }
        }
        free(line);
    }

    free(prompt);
    sh_destroy(sh);
    clear_history();
    return 0;
}
