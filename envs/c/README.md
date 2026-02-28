# C Probe

C is an awful language unless you've already assembled/built yourself an ecosystem.

## What have I learned?

C code can't even developed without a build system.
There's no point developing any C code unless I have a rigorous method for code documentation.
Finally, having the LSP not understand the build compromises my ability to edit the code.

Currently, I have a basic build system, but I know it needs to be pushed further.
I've gotten Doxygen to do a hello-world, but I need to configure it better.
Major TODOs:
- [x] add doxygen to justfile
- [ ] configure clang LSP
- [ ] build archives for static library distribution
- [ ] system to build with static linkage
- [ ] document my util library with doxygen
- [ ] figure out documentation for vendored libraries
- [ ] configure doxygen
- [ ] configure code styling (clang-format, and anything else)
- [ ] document naming conventions:
    types should be UpCamel, locals and functions lowerCamel, dunno about global variables.
    there shoulld be documented abbreviations (buf = buffer, sl = slice, iter = iterator, &c, even the basic ones)
- [ ] setup Doxygen for both api docs and developer docs
more minor:
- [ ] a system for identifying the build
    (say a date+time+nonce, or a hash of the (preprocessed?) input)
    (version defines for all the libraries and whatnot)
- [ ] how the heck do I really manage builds on/for other targets/platforms?
    (sure, arch+os, but what about architecture extensions?, or other system libs like posix or gmp?, or drivers like opengl vs vulkan?, or just not even having the cstdlibe or being baremetal?, and so on)



Misc:
- reserved identifiers:
    `/__.*/` and `_[A-Z].*` are reserved in all contexts (for use by the standard) [see][confluence-reserved-ids].
    `_.*` are reserved for identifiers with file scope (presumably for stdlib implementors) [see][confluence-reserved-ids].
    "potentially reserved identifiers" are hard to spot [see][wg14n2572].
    - [ ] TODO follow up on potentially reserved identifiers
- defer statements:
    Clang 22 has support for them with the `-fdefer-ts` flag.
    They can be simulated in GCC 8+ using inner functions and the cleanup attribute.
- alignment of malloc'd pointers:
    they are aligned to the "fundamental alignment", which is equal to `alignof(max_align_t)`.

[confluence-reserved-ids]: https://wiki.sei.cmu.edu/confluence/spaces/c/pages/87152308/DCL37-C.+Do+not+declare+or+define+a+reserved+identifier
[wg14n2572]: https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2572.pdf

What are the processes I need/want for coding in C?
- ergonomics: generate headers
- builds: static lib, dynamic lib, executable
- builds: native, cross-compile, wasm
- builds: hide symbols in libraries after they've been internaly linked
- style: format checker, auto-format
- api/docs: a solid optparse-appicative-like, esp if it gens man pages, and gives info on return types, envvars, pipes, and so on
- docs: api docgen
- docs: a way to extract and document conventions and patterns from the code, and hopefully link to those patterns where they're used and find their usages
- docs: literate programming
- docs: architecture docs tied to code as much as possible
- docs/testing: tested usage examples
- testing: unit, integration?, golden testing
- security/testing: fuzzing
- security: ubsan, asan/valgrind, and the like
- security: compiler warnings, linter
- security: MISRA coverage and the like, Cert C
- issue tracking: look for `todo` and friends
- debugging: `printf(stderr`, a debugger (gdb, [raddb][raddb])
- modularity: figure out standalone/nolibc/alternate libc builds
- modularity: some way to restrict usage of imported names (ie somewhere `stdlib` is imported, but I want to ensure I don't use `getenv`)
- modularity: newtype/nominal type alias pattern
- reuse: interface pattern, generic data structures
- portability: detect compiler-specific limitations, though I can at least ship with nix

It looks like Doxygen is the only game in town.
`makeheaders` can purportedly extract apis with docs, so perhaps I can get it to work with doxygen.
I already have clang-format, I just need to develop a workflow and continuousy tune it.

[raddb]: https://github.com/EpicGamesExt/raddebugger

From what I've seen, no one has implemented C-syntax `defer` with C++.
You _can_ use lambdas, but ppl are only defining `defer(...)` instead of `defer {...}`.
In short, I don't think I can get C-with-someC++-and-also-C-like-defer.
But what if I could?
- Forward-declare the lambda, give a scope_guard-like unique pointer, then define the lambda after, like jgustadt's gnu C approach?
  Turns out, lambdas can't really be given a type, and their declaration cannot be separate from their definition.
- Try to use the gnu C syntax extensions in `g++` anyway?
  Nope, it looks like GCC doesn't allow nested functions in C++.
  Indeed, [someone says as much](https://forum.arduino.cc/t/nested-functions-sunfunctions-are-they-allowed/132450/8):
    `gcc` has them, but not `g++`.
- Inline classes? Turns out it's easier than you'd think to group initialization with deinitialization.
  See [this SA answer](https://stackoverflow.com/a/7762286), but still I don't think there's a way to write it with C-like syntax.
  Basicaly, I'd still have to define the destructor outside the class, which ig GCC doesn't allow.

## Utility Library

- [ ] TitleCase types
    - C{Int,Long,Uint,...}
    - `{I,U}{8,16,32,64,128?,Ptr,Max}`
    - `Sz`, `PtrDiff`
    - `Byte = unsigned char`, `AsciiChar = char`, `UniChar = uchar32_t`
- [ ] fixed memory
    - [ ] refcounted buffers
    - [ ] slices into refcounted buffers
    - [ ] slices into c buffers
- [ ] tagged pointers
- [ ] dynamic memory
    - [ ] `da` dynamic array (realloc-y)
    - [ ] `xa` exponential array
    - [ ] ring buffer
    - [ ] unsorted dynarray, options to immediate compact or not
    - [ ] block allocator, ig
- [ ] scratch allocator?
- [ ] text types
    - use icu?
    - [ ] utf8 buffer
    - [ ] ascii buffer
- [ ] generic stream type
