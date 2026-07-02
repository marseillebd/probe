// Copyright 2026 - 2026, Marseille Bouchard
// SPDX-Liense-Identifier: LGPL-3.0-or-later

#define A_NOPREFIX 1
#include "a.h"

#include <stdio.h>

void ex_version() {
  printf("%d.%d.%d.%d\n%s\n%d-%d-%d\n"
    , A_VERSION_PROJECT, A_VERSION_MAJOR, A_VERSION_MINOR, A_VERSION_PATCH
    , A_VERSION
    , A_RELEASE / 10000, A_RELEASE / 100 % 100, A_RELEASE % 100
  );
}

void ex_defer() {
  printf("Hello");
  defer {
    defer { printf(", and good luck!\n"); }
    printf("Goodbyte");
  }
  printf(", world!\n");
}

void ex_lengthof() {
  static char answer[42];
  static char invalpha[137];
  debug(lengthof(answer));
  debug(lengthof(invalpha));
}

void ex_pass() {
  pass;
}

void ex_alup() {
  // sweep the size
  printf("alup(10--20, 16):");
  for (int i = 10; i <= 20; i++) {
    printf(" %ld", alup(i, 16));
  }
  printf("\n");
  // sweep the alignment
  printf("alup(41, 2^(0--8)):");
  for (int i = 0; i <= 8; i++) {
    printf(" %ld", alup(41, 1 << i));
  }
  printf("\n");
}

void ex_talloc() {
  void* talloc_0 = talloc(0) + FUND_ALIGN; // add the fundamental alignment bc a zero-size alloc is min that size
  void* talloc_3 = talloc(3);
  debug(talloc_3 - talloc_0);
  {
    A_Savepoint save = tallocSave();
    defer { tallocReset(save); }
    printf("save\n"); defer { printf("restore\n"); }
    void* talloc_512 = talloc(512);
    debug(talloc_512 - talloc_0);
    void* talloc_next = talloc(0);
    debug(talloc_next - talloc_0);
  }
  void* talloc_32 = talloc(32);
  debug(talloc_32 - talloc_0);
  void* talloc_next = talloc(0);
  debug(talloc_next - talloc_0);
}

#define ex(name) \
  printf("====== " #name " ======\n"); \
  ex_##name()

int main() {
  ex(version);
  ex(defer);
  // ENSURE/ensure, isArray
  ex(lengthof);
  // match
  ex(pass);
  // noreturn
  // impossible
  // stringize, token paste
  // all the type names? yeah, and size+align
  // is_pow2
  ex(alup);
  // TODO mkpow2
  // binary SI prefixes
  ex(talloc);
  // TODO more!

  printf("============ OK ============\n");
  return 0;
}
