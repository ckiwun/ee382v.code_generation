#include <stdio.h>
#include <stdlib.h>
#include "epp_runtime.h"

int
main(int argc, char **argv) {
  	for (int i = 0, e = atoi(argv[1]); i < e; ++i) {
  	  if ((i + argc) % 3) {
  	    printf("For1 Truey\n");
  	  } else {
  	    printf("For1 Falsey\n");
  	  }
  	}
  	for (int i = 0, e = atoi(argv[1]); i < e; ++i) {
  	  if ((i + argc) % 2) {
  	    printf("For2 SKIPPED\n");
		continue;
  	  } 
  	    printf("For2 DOIT\n");
  	 
  	}
  dump_path_regs();
  return 0;
}
