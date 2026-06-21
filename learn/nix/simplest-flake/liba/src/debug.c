#define A_NOPREFIX 1
#include "a.h"

#include <stdio.h>

int main() {
  printf("Hello\n");
  defer { printf("goodbyte\n"); }
  void* foo = a_talloc(clamp(0, 3103, 3));
  a_debug(foo);
  void* nxt = a_talloc(0);
  a_debug(nxt);
  long long asdf = 4;
  a_debug(asdf);
  return 0;
}
