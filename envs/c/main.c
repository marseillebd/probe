#include "mbd/util.h"

#include "lib.h"

#define show(label, format, expr) \
  printf(label" = "format"\n", (expr))
#define showDec(expr) \
  show(STR(expr), "%d", (expr))

struct Entry {
  const char* name;
  size_t s;
  size_t a;
};
struct Entry entries[] =
  {
#define E(ty) (struct Entry){ STR(ty), sizeof(ty), alignof(ty) },
#define G(name) (struct Entry){(name), 0,0 },
    G("basic types")
    E(char)
    G("limit types")
    E(max_align_t)
    E(intmax_t)
    E(uintmax_t)
    G("fast int types")
    E(uint_fast8_t)
    G("floating-point types")
    E(float)
    E(double)
    E(long double)
    E(__float128)
#undef E
  };

#include <string.h>
int main() {

  size_t max_name_len = 0;
  for (int i = 0; i < sizeof(entries)/sizeof(struct Entry); ++i) {
    if (entries[i].s == 0) { continue; }
    size_t len = strlen(entries[i].name);
    if (len > max_name_len) { max_name_len = len; }
  }

  show("CHAR_BIT", "%d", CHAR_BIT);
  for (int i = 0; i < sizeof(entries)/sizeof(struct Entry); ++i) {
    struct Entry entry = entries[i];
    if (entry.s != 0) {
      printf("%-*s | %3d %3d\n", (int)max_name_len, entry.name, (int)entry.s, (int)entry.a);
    } else {
      printf("\n%s:\n", entry.name);
    }
  }

  hello();
  return 0;
}
