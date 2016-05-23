/* Borrowed from Wikipedia: Function prototype page. */
#include <stdio.h>

int myfunction(int n);

int main(void) {
  printf("%d\n", myfunction()); /* Intentional: we want this to fail. */
  return 0;
}

int myfunction(int n) {
  if (n == 0) {
    return 1;
  }
  return n * myfunction(n - 1);
}

