#include <stdio.h>
int n;
void foo();
float n;
void foo() {
  n = 1.5;
}
int main() {
  n = 256;
  foo();
  printf("%d", n);
}