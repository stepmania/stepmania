#include <stdio.h>

int very_long_name_here_one() { return 0; }
int very_long_name_here_two() { return 1; }

int main() {
  printf("%d\n", very_long_name_here_one());
  printf("%d\n", very_long_name_here_two());
  return 0;
}

