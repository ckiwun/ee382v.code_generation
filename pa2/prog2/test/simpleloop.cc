#include <stdio.h>
#include <stdlib.h>
#include "epp_runtime.h"

int
main(int argc, char **argv) {
  for (int i = 0, e = atoi(argv[1]); i < e; ++i) {
    if ((i + argc) % 3) {
      printf("Truey\n");
    } else {
      printf("Falsey\n");
    }
  }

  dump_path_regs();
  return 0;
}
