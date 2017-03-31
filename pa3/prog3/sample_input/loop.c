int x;

int extra(int);
int extra2(int, int);
int f(int);

int f(int s) {
  int i = 0, j = 0;
  while (i < s + 10) {
    int y = extra(i);
    while (j < s)
      j = extra2(y, j);
    i = extra(j);
  }
  return i;
}