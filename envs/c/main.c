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

// TODO move this into mbd (or should I call it catteries? like c + batteries)
#define sgn(x) ((x) > 0) - ((x) < 0)

struct qr { int q, r; };
struct qr quotrem(int n, int d) {
  return (struct qr){.q = n/d, .r = n%d};
}
struct qr divmod(int n, int d) {
  // Why? Ie what are the properties?
  // First, we have $n/d = q + r/d$ in the reals by definition.
  // With a little algebra, we get an _integral_ equation $n = qd + r$.
  // Second, we have that multiplying both inputs by $-1$ does not affect the quotient (ie the quotient function is linear, which I think we should expect from division).
  // The remainder is _not_ linear, but we didn't have that when we met condition 1 with rounding towards zero (C-style) anyway.
  // Third, the range of the remainder is between 0 inclusive and the denominator exclusive.
  // This means that for positive denominators, the remainder is ordinary number-theoretic equivalence modulus the denominator,
  //   whereas the C-style range is the much crunchier between zero inclusive and $sgn(numerator)*abs(denominator)$ exclusive, which I dont' think has applications.
  //
  // Ok, so how does this happen, and why does C pick the mathematically oddball one?
  // Consider division by repeated "subtraction", represented in the following tables.
  // The left column counts how many times we've "subtracted", and the right is initialized with the numerator and "drops" down by the denominator every time.
  // The reason I say "subtract" with the scare quotes is because subtraction makes sense in the first table, which you should focus on first, as it's familiar.
  //
  // ```
  //  5/2    5/-2     -5/2    -5/-2
  // _____  _______  _______  ______
  // 0 | 5   0 |  5   0 | -5  0 | -5
  // 1 | 3  -1 |  3  -1 | -3  1 | -3
  // 2 | 1  -2 |  1  -2 | -1  2 | -1
  //        -3 | -1  -3 |  1
  //
  // In the first table, we just try to bring 5 down towards zero until we can't subtract anymore (without going negative, ie crossing zero).
  // This explains why dividing positives rounds both towards-zero and down.
  // Looknig at the fourth table, the same sort of thing happens, except were's subtracting a negative, and so we're bringing -5 _up_ towards zero.
  // Again, towards zero but without crossing it, so dividing negatives also rounds down and towards zero.
  //
  // The second and third coumns are where things get interesting.
  // What makes them different is that the signs of the numerator and denominator don't match.
  // Now, if we tried to actually subtract the denominator, we would grow _away from zero_, and so reach no boundary.
  // Instead, we have to _add_ the denominator to bring our numerator towards zero, effectively taking _backwards_ steps;
  //   that's why I added the scare quotes around "subtract" earlier.
  // In these columns, we take negative steps, bringing our remainder computation towards zero.
  //
  // Now, the question is: should we continue to _not_ cross zero?
  // 1. If we don't, then division takes less time, and that's almost certainly related to why silicon makes this choice.
  //    However, it means the quotient rounds towards zero, which in these cases is _up_ instead of _down_ from the other columns.
  //    The difference in direction of rounding makes the quotient function non-linear.
  // 2. If we _do_ cross zero, we need to take an extra step, costing time.
  //    The benefit is that the quotient is linear, and the remainder is now the number-theoretic _modulus_, even when the numerator is negative.
  //    If that second point is a bit abstract, it means that a test like `x % 2 == 1` correctly detects when an integer is odd; it would not under option 1.
  //    In summary, a little more time, much better mathematical properties.
  // ```
  auto qr = (struct qr){.q = n/d, .r = n%d};
  if (sgn(n) != sgn(d)) { qr.q -= 1; qr.r += d; }
  return qr;
}

// Hey btw, subtraction is repeated predecessor, and division is repeated subtraction;
// how come we don't have repeated whatever to inverse the definition of exponentiation as repeated multiplication?
// Well, addition and multiplication are commutative, which means their left incerse is identical to their right inverse.
// Exponentiation is _not_ commutative, so it can have different left- and right-inverses.
// In particular, nth-root cancels nth-power, and log-base-n cancels powers-of-n.
// It might even have something to do with the general exp function being a matter of multiplying by a pseudo-scalar rather than a simple scalar (as understood under geometric algebra).

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

  //quotrem vs divmod
  printf("============\n");
  printf(
    "numer/denom | quot, rem | n == d*q+r\n"
    "            |  div, mod | n == d*q+r\n"
  );
  for (int d = 2; d >= -2; d-=4) {
    for (int n = 1; n >= -1; n-=2) {
      printf("-----------------------------------------\n");
      struct qr qr = quotrem(n, d);
      printf("%5d/%-5d | % 4d, % -3d |  % d == % d\n", n, d, qr.q, qr.r, n, d*qr.q+qr.r);
      qr = divmod(n, d);
      printf("            | % 4d, % -3d |  % d == % d\n", qr.q, qr.r, n, d*qr.q+qr.r);
    }
  }

  hello();
  return 0;
}
