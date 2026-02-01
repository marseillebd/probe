// I spotted something odd in the C standard the other day:
// an `identifier` has (paraphrasing) the grammar
// `identifier-nondigit ::= /[_a-zA-Z]|\\u\h{4}|\\U\h{8}/`.
// That is, we can use unicode escapes in identifier tokens?
// Yes, and this file is a demo.
//
// There are some wrinkles. First, the standard also states the codepoint cannot be:
// - less than U+00A0 (in ascii or the C1 control codes),
//   unless it's U+0024 `$`, U+0040 `@`, or U+0060 (backtick).
// - in the range U+D800 -- U+DFFF inclusive (surrogate pairs)
// - greater than U+10FFFF (larger than what unicode says they'll ever allow)
//
// Second, gcc 15.1.0 does not allow it under `-std=c23`.
// I got this to compile with `gcc -std=c99 test.c`,
// and you can verify with `objdump -d a.out`, which displays:
//
// ```
// 0000000000401160 <fooλ>:
//   401160:	48 8d 05 9d 0e 00 00 	lea    0xe9d(%rip),%rax        # 402004 <_IO_stdin_used+0x4>
//   401167:	c3                   	ret
// ```
//
// I haven't devoted the time to even figure out exactly what language versions this works for.
// Perhaps I should, and submit a patch?

// First, an ordinarily-named function `ok`.
char* ok() {
  return "Hello";
}

// Then, a similar function, but named `fooλ` defined with a UCN (universal character name).
// To not have to write the escape every time, I've also added the `odd` macro.
#define odd foo\u03bb
char* odd() {
  return "lambda";
}

#include <stdio.h>
int main() {
// Sure enough, this prints `Hello, lambda!`.
  printf("%s, %s!\n", ok(), odd());
  return 0;
}

