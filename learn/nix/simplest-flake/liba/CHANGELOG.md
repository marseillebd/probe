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


v 1.0.0.2-alpha
---------------

- TODO: audit for abbreviations
- TODO: distribution scripts
- TODO: document build script
- TODO: documentation in pdf

- Changed naming convention format.
- Added naming convention for conversions.
- Added more default headers: stdlib, unistd.
- Changed the `Sz`, `PtrDiff` set to `USz`, `ISz`, `SSz`, with more semantic implications.
- Added `CStr`, adding intention to `char*`.
- New module: `Bytes` aka `Bs` for (technically mutable) slices into byte strings.
- New module: `Asciiz`, which is `Bytes` but with the intended restriction to ascii strings that can be nul-terminated (ie contain no NUL characters).
- Added smoke tests using nix.
- Added a license and applied spdx headers.
- Changed build scripts to export building functions to be used from the flake (or user on the commandline).
- Added documentation in html format.

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
