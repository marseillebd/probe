Later
-----

- New helper macro for defining inline functions.
  Maybe like `INLINE(LIBNAME)` gets defined based on `LIBNAME_IMPLEMENTATION`.
- syntax for tail calls, out parameters
- prefix/suffix/infix operations on `Bytes`
- readline (and chomp) for `Bytes` (and likelyalso helpers for `Asciiz`
- wrap OS interface to accept `Bytes`, `Asciiz`, and/or `Text` (unicode) where relevant

v 1.0.0.2-alpha
---------------

- Changed naming convention format. (TODO audit)
- New naming convention for conversions. (TODO document, audit)
- Added more default headers: stdlib, unistd.
- Change basic type names to be lowercase, matching new naming conventions.
- Changed the `Sz`, `PtrDiff` set to `USz`, `ISz`, `SSz`, with more semantic implications.
- Added `CStr`, adding intention to `char*`.
- New module: `Bytes` aka `Bs` for (technically mutable) slices into byte strings.
- New module: `Asciiz`, which is `Bytes` but with the intended restriction to ascii strings that can be nul-terminated (ie contain no NUL characters).


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
