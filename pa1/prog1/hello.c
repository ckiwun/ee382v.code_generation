#include <stdio.h>

int a[10];

int factor(int n) {
  int i, ret = 1;
  for (i = 1; i <= n; i++)
    ret *= i;
  return ret;
}

int main() {
  int i;
  for (i = 0; i < 10; i++) {
    if (i == 0)
      a[i] = 1;
    else 
      a[i] = factor(i);
  }
    
  printf("factor[9]= %d\n", a[9]);
  return 0;
}
