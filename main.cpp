#include <stdio.h>
#include <readline/readline.h>
#include "lab.h"

int main(int argc, char **argv)
{
    char *line = (char *)NULL;
    line = readline("What is your name?");
    printf("Hello %s! This is the starter template version: %d.%d\n", line, lab_VERSION_MAJOR, lab_VERSION_MINOR);
    return 0;
}
