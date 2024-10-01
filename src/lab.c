#include "lab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <readline/history.h>
#include <signal.h>
#include <pwd.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_JOBS 100

struct job {
    int job_number;
    pid_t pid;
    char *command;
    int status;
};

struct job jobs[MAX_JOBS];
int next_job_number = 1;

void update_job_status(pid_t pid);
void print_jobs();
void add_job(pid_t pid, int job_number, char **argv);
void check_background_processes();
void remove_completed_jobs();

char *get_prompt(const char *env) {
    const char *prompt_env = getenv(env);
    char *prompt;
    size_t prompt_length;

    if (prompt_env != NULL) {
        prompt_length = strlen(prompt_env) + 1;
    } else {
        const char *default_prompt = "shell>";
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

    if (dir[1] == NULL) {
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
        new_dir = dir[1];
    }

    if (chdir(new_dir) != 0) {
        perror("cd");
        return -1;
    }

    return 0;
}

char **cmd_parse(char const *line) {
    char **argv = NULL;
    char *token;
    char *line_copy = NULL;
    int argc = 0;

    long max_args = sysconf(_SC_ARG_MAX);
    if (max_args == -1) {
        perror("sysconf");
        return NULL;
    }

    argv = malloc((max_args + 1) * sizeof(char *));
    if (argv == NULL) {
        perror("malloc");
        return NULL;
    }

    line_copy = strdup(line);
    if (line_copy == NULL) {
        perror("strdup");
        free(argv);
        return NULL;
    }

    trim_white(line_copy);

    token = strtok(line_copy, " ");
    while (token != NULL && argc < max_args) {
        argv[argc] = strdup(token);
        if (argv[argc] == NULL) {
            perror("strdup");
            for (int i = 0; i < argc; i++) {
                free(argv[i]);
            }
            free(argv);
            free(line_copy);
            return NULL;
        }
        argc++;
        token = strtok(NULL, " ");
    }

    argv[argc] = NULL;
    free(line_copy);
    return argv;
}

void cmd_free(char **line) {
    if (line == NULL) {
        return;
    }

    for (int i = 0; line[i] != NULL; i++) {
        free(line[i]);
    }

    free(line);
}

char *trim_white(char *line) {
    if (line == NULL) {
        return NULL;
    }

    char *start = line;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') {
        *line = '\0';
        return line;
    }

    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';

    if (start != line) {
        memmove(line, start, (end - start + 2) * sizeof(char));
    }

    return line;
}

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
        return change_dir(argv);
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

    if (strcmp(argv[0], "jobs") == 0) {
        print_jobs();
        return true;
    }

    pid_t pid = fork();
    int background = 0;
    
    for (int i = 0; argv[i] != NULL; i++) {
        if (argv[i+1] == NULL && strcmp(argv[i], "&") == 0) {
            background = 1;
            argv[i] = NULL;
            break;
        }
    }

    if (pid == -1) {
        perror("fork");
        return false;
    } else if (pid == 0) {
        pid_t child = getpid();
        setpgid(child, child);
        
        if (!background) {
            tcsetpgrp(sh->shell_terminal, child);
        }

        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);

        execvp(argv[0], argv);
        fprintf(stderr, "exec failed\n");
        exit(1);
    } else {
        setpgid(pid, pid);
        
        if (!background) {
            tcsetpgrp(sh->shell_terminal, pid);
            int status;
            waitpid(pid, &status, WUNTRACED);
            tcsetpgrp(sh->shell_terminal, sh->shell_pgid);
        } else {
            add_job(pid, next_job_number, argv);
            printf("[%d] %d\n", next_job_number, pid);
            next_job_number++;
        }

        return true;
    }
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

        signal(SIGINT, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
    }

    using_history();

    for (int i = 0; i < MAX_JOBS; i++) {
        jobs[i].pid = 0;
        jobs[i].command = NULL;
    }
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

void check_background_processes() {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        update_job_status(pid);
    }
}

void print_jobs() {
    int jobs_found = 0;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].pid != 0) {
            jobs_found = 1;
            if (jobs[i].status == 0) {
                if (kill(jobs[i].pid, 0) == 0) {
                    printf("[%d] %d Running %s\n", jobs[i].job_number, jobs[i].pid, jobs[i].command);
                } else {
                    printf("[%d] Done    %s\n", jobs[i].job_number, jobs[i].command);
                    jobs[i].status = 1;
                }
            } else {
                printf("[%d] Done    %s\n", jobs[i].job_number, jobs[i].command);
            }
        }
    }
    if (!jobs_found) {
        printf("No active jobs\n");
    }
}

void add_job(pid_t pid, int job_number, char **argv) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].pid == 0) {
            jobs[i].job_number = job_number;
            jobs[i].pid = pid;
            jobs[i].status = 0;
            
            char command[1024] = "";
            for (int j = 0; argv[j] != NULL; j++) {
                strcat(command, argv[j]);
                strcat(command, " ");
            }
            strcat(command, "&");
            jobs[i].command = strdup(command);
            
            break;
        }
    }
}

void update_job_status(pid_t pid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].pid == pid) {
            jobs[i].status = 1;
            break;
        }
    }
}

void remove_completed_jobs() {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].pid != 0 && jobs[i].status == 1) {
            free(jobs[i].command);
            jobs[i].pid = 0;
            jobs[i].command = NULL;
            jobs[i].status = 0;
        }
    }
}