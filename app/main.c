#include <stdio.h>
#include "../src/lab.h"

int main(int argc, char **argv)
{
  parse_args(argc, argv);
  get_prompt(argv[0]);
  return 0;
}
