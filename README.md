A dedicated repository for me to play around in, filled with tools already set up.

Usage
=====

Development Environment
-----------------------

If you haven't already, install nix and enable flakes:
```
sh <(curl --proto '=https' --tlsv1.2 -L https://nixos.org/nix/install) --daemon
sudo echo 'experimental-features = nix-command flakes' >> /etc/nix/nix.conf
```

TODO: d/l and enter the repo, maybe init submodules, _and then_ enable the development shell

Enter the development shell:
```
nix develop
```

TODO: mention flake update, garbage collection, and whatever

Play Around
-----------

TODO: branch to a topic branch from the latest main, do not merge it back in
TODO: if you upgrade the repo, I suggest splitting the git history with rebase, I think

Developing the Probe Repo
-------------------------

TODO how to develop in the probe repo

TODO List
=========

Follow-up
---------

- [ ] [{extra,}{native,}BuildInputs](https://discourse.nixos.org/t/extrabuildinputs-extranativebuildinputs/30664/2)
- [ ] [replology](https://repology.org/) for tracking where my project is packaged

Utilities
---------

- [ ] rlwrap
- [ ] tree, sponge, jq, yq?, watch, smth to get notified on filesys changes
- [ ] rg, ag
- [ ] icdiff (perhaps more difftools)
- [ ] copyright management stuff
- [ ] smth to make writing one-off services/daemons easy
- [ ] audit tools from compiler-tools
- [ ] dependency tracking tools

- [ ] a general-purpose literate programming tool
  - evaluate:
    - [ ] [shocco](https://rtomayko.github.io/shocco/) looks really nice,
      but only for langs that use `#` comments and expect a shebang.
      Could be hacked into something more general-purpose fairly easily, good chance to try a _modeled_ state-machine approach.

Stock Code
----------

- [x] [EditorConfig](https://editorconfig.org/)
- [ ] `.gitattributes`
- [ ] sample git hooks
  - [ ] eclint before commit
  - [ ] look into the uses of other git hooks
  - [ ] shellcheck, shfmt before commit
  - [ ] run test cases before push?

Structural
----------

- [ ] gui git tool so I can look at the different experiments
- [x] learn directory, with reference resources for various environs/tools
- [ ] utilities directory, for little custom tools that aren't packaged separately

Environments
------------

- [ ] c (incl gcc, clang, tinycc?, certc?)
    - evaluate:
        - [ ] [CCAN - C Code Archine Network](https://ccodearchive.net/)
        - [x] [zpl](https://github.com/zpl-c/zpl): too general/portable, but good inspo
        - [ ] [arena](https://github.com/daddinuz/arena)
        - [ ] defer TS on gcc and clang
        - [ ] a set of opengl helpers
        - [ ] [stb](https://github.com/nothings/stb)
          - the lexer might be a nice thing to have
        - [ ] [clib](https://github.com/clibs/clib): honestly, probly wouldl just use nix as my package manager
        - [ ] [nob](https://github.com/tsoding/nob.h)
        - [ ] [raylib](https://github.com/raysan5/raylib)
        - [ ] [libffi](https://github.com/libffi/libffi)
- [ ] hs
- [ ] md
- [ ] nix
- [ ] rust
- [ ] elf
- [ ] x64, x86, other asms
- [ ] js, ts
- [ ] python
- [ ] taskfiles
- [ ] vimscript
- [ ] data formats: jsoon, toml, sexpr, cookie-jar
- [ ] I could be convinced to try ada

Sh/Bash
-------

- [ ] posix sh

- [x] use devshell bash
- [x] template
- [x] shellcheck
- [x] style formatter: shfmt
- [ ] references
  - [ ] shell & bash bibles
  - [ ] gnu bash reference
  - [x] style.yasp.sh refere
- [ ] [shellharden?](https://github.com/anordal/shellharden/)
- [ ] testing
  - evaluate:
    - [ShellSpec](https://shellspec.info/)

- [ ] how do I search for an ancestor file/directory matching some condition? like how git finds the repo by looking for a parent with a .git folder?

