#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "../src/lab.h"

int main(int argc, char **argv)
{
  parse_args(argc, argv);

  char *prompt = get_prompt("MY_PROMPT");
  if (prompt == NULL) {
      fprintf(stderr, "Failed to get prompt\n");
      return 1;
  }

  using_history();
  char *line;

  while ((line = readline(prompt))) {
      if (strlen(line) > 0) {
          add_history(line);
          printf("%s\n", line);
      }
      free(line);
  }

  free(prompt);
  return 0;
}
