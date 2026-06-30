Later
-----

- take version numbers from the flake
- build system enables link-time-optimization, so I don't feel bad about not providing feature flags
- New helper macro for defining inline functions.
  Maybe like `INLINE(LIBNAME)` gets defined based on `LIBNAME_IMPLEMENTATION`.
- syntax for tail calls, out parameters
- prefix/suffix/infix operations on `Bytes`
- readline (and chomp) for `Bytes` (and likelyalso helpers for `Asciiz`
- wrap OS interface to accept `Bytes`, `Asciiz`, and/or `Text` (unicode) where relevant
- move header imports from a.h to a.c as much as possible
- technique: can I use x-macros to compile pattern-amtching to switches?
- technique: perhaps I define macros that can emit parts of `_Generic`, and then a c file just puts together the generic it needs
- documentation needs ToC
- move building of tests from liba, and into the tests package?

v 1.0.0.3-alpha
---------------

- TODO: distribution scripts
- TODO: document build script
- TODO: documentation in pdf

- Added `ENSURE`/`ensure`, `isArray` to support more type checking in `lengthof`.
- Added `match` syntax to help with pattern-matching.
- Added `impossible()` syntax.
- Added 2-/3-D vectors and dot products, an experiment in eventually implementing geometric algebra.

v 1.0.0.2-alpha
---------------

- Changed naming convention format.
- Changed the `Sz`, `PtrDiff` set to `USz`, `ISz`, `SSz`, with more semantic implications.
- Added naming convention for conversions.
- Added more default headers: stdlib, unistd.
- Added `CStr`, adding intention to `char*`.
- Added `Bytes`/bs module for (technically mutable) slices into byte strings.
- Added `Asciiz`/az module, which is `Bytes` but with the intended restriction to ascii strings that can be nul-terminated (ie contain no NUL characters).
- Added smoke tests using nix.
- Added a license and applied spdx headers.
- Changed build scripts to export building functions to be used from the flake (or user on the commandline).
- Added documentation in html format.
- Updated interface documentation for style.
- Fixed `cmp_bs` returning a bool instead of an int.

v1.0.0.1-alpha
--------------

- version metadata, ofc
- keywords `pass`, `defer`, `noreturn`
- a bunch of default headers
- stringize and token paste helper macros
- normalized (enshrined? modernized?) names for basic types
- `A_NOPREFIX` strips the liba prefix off where it makes sense
- For some reason, macros that multiply by binary SI sizes: `B`, `KiB`, `MiB`, `GiB`
- Temporary allocator `talloc` with save and restore functions.
- `a_debug` which `printf`s a value to `stderr`, with its name/expression, and trys to choose a good encoding (works well for basic types)
- the silly little helpers: `max`, `min`, and `clamp`. also, `ver` for fun
