# Questions

- [ ] how to build/develop on external dependencies (like a patched raylib)
- [ ] nix building phases
  - [ ] there's a `check` phase; is that a good place to run tests? if not, then where?
- [ ] `inputsFrom` in a derivation
  - [ ] is there a `nativeInputsFrom`?
  - [ ] how does it interact with `buildInputs`?
  - [ ] can I merge multiple `inputsFrom`?
- [ ] swapping out my c compiler (ie not use mkDerivation)
- [ ] can I get the `nix run` CLI to run a different exe by name?
- [ ] how can I use nix to build different distributions? (source, binary, documentation, single-file-header, &c)
- [ ] in nix bash script, `$flags[@]` worked fine, but in real bash I had to use `${flags[*]}`; why?


# Main Commands

```sh
nix flake init
nix {build,run} [ .#<package> ]
nix build .#<package>-<tests>
nix develop [ .#<devShell> ]
nix log $(realpath result) # for successful builds
```

`nix run` will look for a binary `$out/bin/$pname` (ie the exe which is named the same as the package).

There may be a better way, like a mythical `nix build --check` with a `checkPhase`, but:
  I haven't gotten `checkPhase` to find the output from `buildPhase` (might just be a variable name I don't know),
  and anyway, there might be several levels of testing.
For now, I'm creating a "dummy" package who's build phase just runs the tests that were distributed in the main package.
I got this strategy from [pfassina](https://deepwiki.com/pfassina/lazyvim-nix/7.3-running-and-writing-tests).

# What I've learned

Project organization can be tricky.
I'd like to build/dist/run with nix, but downstream might not be using nix.
The need to provide builds via both nix and not makes it much more appealing to use a set of build scripts.
The scripts should themselves have a `#!/usr/bin/env <lang>`, but nix needs to call them via the `<lang>` interpreter's command.
At least this is the case with `stdenv.mkDerivation`+`bash`, and I'm not really sure why.

I think I've got a fairly nice generic build script for small projects.
It needs some maturing, but it works for `liba` at least, and shouldn't be hard to parameterize.
- [ ] follow up on the simple c project build script

It turns out that STB-style single-file headers are really easy to create from a source/header.
That in turn makes them much easier to develop, because I don't have to tell gcc that an h file is actually a c file.

I'm not sure, but what if we did something like the folloing to reduce code duplication for exported inline functions.
```c
// In the header file:

#define inlinebody__myfunc {  \
   if (a == b) { return; }    \
   while (*a) *b++ = *a++;    \
}
extern inline
void myfunc(char* a, char* b) inlinebody__myfunc

// In the source file
#include <foo.h>
inline
void myfunc(char* a, char* b) inlinebody__myfunc
```

# Bibliography

## https://venikx.com/posts/how-to-nix/

Nice guide that really builds up step-by-step for the very beginner.
Covers `nix shell`,
  getting set up with flakes,
  `nix develop` and devShells,
  `nix build` and `nix run`
