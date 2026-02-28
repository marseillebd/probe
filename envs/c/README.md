# C Probe

C is an awful language unless you've already assembled/built yourself an ecosystem.

# What have I learned?

C code can't even developed without a build system.
There's no point developing any C code unless I have a rigorous method for code documentation.
Finally, having the LSP not understand the build compromises my ability to edit the code.

Currently, I have a basic build system, but I know it needs to be pushed further.
I've gotten Doxygen to do a hello-world, but I need to configure it better.
Major TODOs:
- [x] NEXT add doxygen to justfile
- [x] NEXT configure clang LSP
- [ ] NEXT document my util library with doxygen
- [ ] configure doxygen
  - [ ] NEXT developer docs
  - [ ] figure out documentation for vendored libraries
  - [ ] api docs
- [ ] NEXT configure code styling (clang-format, and anything else)
- [ ] build archives for static library distribution
- [ ] system to build with static linkage
- [ ] document naming conventions:
    types should be UpCamel, locals and functions lowerCamel, dunno about global variables.
    there shoulld be documented abbreviations (buf = buffer, sl = slice, iter = iterator, &c, even the basic ones)
more minor:
- [ ] a system for identifying the build
    (say a date+time+nonce, or a hash of the (preprocessed?) input)
    (version defines for all the libraries and whatnot)
- [ ] how the heck do I really manage builds on/for other targets/platforms?
    (sure, arch+os, but what about architecture extensions?, or other system libs like posix or gmp?, or drivers like opengl vs vulkan?, or just not even having the cstdlibe or being baremetal?, and so on)
later majors:
- [ ] research and create recipes for distribution: src-release, pre-compiled releases, packages (debian, arch, nix, alpine?, &c), dev packages (ie header and libraries), &c.

## Flag Management

Both gcc and clang support `@<file>` syntax to include arguments from a file (separated only by lines).
This allows me to share the config options between `clangd` and the `clang` invocation in `just build`.

Supposedly, the `@<file>` syntax is also supported inside an included file.
That would be nice to separate out, say `dialect-{gcc,clang}`, `diagnostics`, `debug`, `release`, and so on.

## LSP Support

It seems clangd is the only reasonable game in town.
Perhaps there are other LSPs, but they follow the clangd interface.

Without configuration, my vim's basic LSP client won't produce good diagnostics.
In particular, it doesn't understand where to find (project and vendored?) headers.
Also, it will need to understand the language dialect, diagnostics, and perhaps also style.

Where does this configuration go?
- [Json Compilation Database][]
  - `compile_flags.txt` holds cli options, one per line.
    This is simple enough to hand-write, but is not flexible[^txt-flex].
  - `compile_commands.json` is more annoying to hand-write, and the "specification" includes surprisingly little information.
    So, flexible, not cannot be hand-written.
    Opinions online overwhelmingly prefer this to `complile_flags.txt`, perhaps because they are thinking of large projects.
- [`.clangd` Configuration][] Doesn't seem to come up much in discussions, but unfortunately is the top result when you search for "clangd configuration".
  It seems like it's reasonably possible to hand-write.
  There's not much info on how a `.clangd` file interacts with `compile_{flags.txt,commands.json}`, but I did find [this][clangd vs commandsjson].
  Apparently, is can be nice to add extra diagnostics.

How do I create these configs?
The only one I really need to generate it seems is `compile_commands.json`.
- For simple projects, there's [Bear][].
- The most popular way is to use cmake to generate it.
  Of course, cmake is a heavy tool, capable of generating lots of stuff, but that does make it useful when a project grows.
  If you're using cmake already, it's a no-brainer, one-liner.
  If not, there's a [hacky use of cmake to generate the commandsjson][], which seems simple enough.

So, how am I configuring my LSP?
- [ ] since I'm writing a basic probe, I'll just use a hand-written `compile-flags.txt`.
  - I'm not sure it's that useful to explore [Bear][], but if I have issues with cmake, then perhaps.
  - [ ] only later will I integrate cmake, which seems to be a popular tools for larger, more complex projects.
- [ ] LATER there's [this](https://github.com/clangd/clangd/discussions/2489) which vaguely describes a setup at least one person finds useful

[^txt-flex]: doesn't support different flags for different translation units, build profiles, or other parameters. \
  This usually means it can't handle existing complex projects.

[Json Compilation Database]: https://clang.llvm.org/docs/JSONCompilationDatabase.html
[`.clangd` Configuration]: https://clangd.llvm.org/config
[clangd vs commandsjson]: https://github.com/clangd/clangd/discussions/1985

[Bear]: https://github.com/rizsotto/Bear
[hacky use of cmake to generate the commandsjson]: https://gist.github.com/Strus/042a92a00070a943053006bf46912ae9

### Getting the LSP to work on NixOS

I'm writing this on 2026-02-28, and while `clang` successfully found system headers, `clangd` was not.
In Nix, `clang` is actually a wrapper script that injects a bunch of Nix-specific flags (including where to look for system headers).
However, the clangd from the `clang` package was not likewise wrapped.
Thankfully, the clang from the `clang-tools` package _is_ wrapped.
I reportedly have to put `clang-tools` before `clang` in the package list, but no reason was given;
    I suspect the ordering determines the ordering of the `PATH`, and thus which is used.

References:
- [This post](https://discourse.nixos.org/t/get-clangd-to-find-standard-headers-in-nix-shell/11268/13) walks through some debugging steps.
  Of import: running `NIX_DEBUG=1 clang ...` will dump info about what flags the wrapper introduces
- [this PR](https://github.com/NixOS/nixpkgs/pull/120229) discusses the wrapper improvements made in clang-tools
- [This thread](https://discourse.nixos.org/t/clang-clang-and-clangd-cant-find-headers-even-with-compile-commands-json/54657) has a lot of talk, but I'm not sure how much of it really gets into what's realy going on.

## Misc
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

# Goals

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
- Forward-declare the lambda, give a `scope_guard`-like unique pointer, then define the lambda after, like jgustadt's gnu C approach?
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
- [ ] interfaces:
    - first, the kinds of interfaces and data structures
      - `-Arr` which is an immutable array in contiguous memory, prolly refcounted.
      - `-Sl` which is a slice into an `Arr`.
      - `-Buf` which are append-only, usually a da or xa. Convert to a stream or an array, or perhaps even a builder.
      - `-Bldr` which is a builder: a tree of references to sized streams??
      - `-S` which is a stream
    - crossed with the sorts of elements
      - `Byte` for bytes
      - `Utf8` for utf-8 encoded unicode (backed by a `ByteX`)
      - `Ascii` for 7-bit ascii
    - and we have some special types, I think just streams:
      - `LineS` for "plaintext" utf-8/ascii, with info about the line ending, possibly with position info
      - `Utf8DecodeS` for iterating over the tagged sum of valid sequences of utf8 with utf8 encoding errors
