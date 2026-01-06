Derivations
===========

When the source is already unpacked in the same directory as the derivation
---------------------------------------------------------------------------
In `mkDerivation`, use `src = ./.` (without quotes?).
Then, also pass `dontUnpack = true`.
Also, save some build time/space by using `stdenvNoCC.mkDerivation`, since no c compiler is needed.

- [ ] look into the perhaps more principle appraoch on [nix.dev](https://nix.dev/tutorials/working-with-local-files.html#union-explicitly-include-files)

References:
- [create a derivation in nix to only copy some files](https://stackoverflow.com/q/76242980)
- [Deprecate explicit setting of phases](https://github.com/NixOS/nixpkgs/issues/28910)

Dependency Graph
================

Surely useful for SBOMs.
Certainly useful for tracking what I actually depend on.

- transitive dependency tree: `nix-store -q { --requisites | --tree | --graph } <paths...>`
  Prints other store paths that the given path depends on.
  Choose `--tree` for ASCII, `--graph` for input to graphviz's `dot` command, `--requisites` for a flat list.
  The `paths...` are store paths (`/nix/store/<hash>-<artifact>`).
  For an executable, use `$(which <exe>)` as the path.
  For libraries, find an exe that links to it, then `$(ldd $(which <exe>) | grep <library> | cut -d ' ' -f 3)` is the path.
  For build-time dependencies, use `$(nix-store -q --deriver <store path>)`.
- The tools in [sbomnix] seem a bit more integrated, though quite slow.
  - transitive dependencies:
    Usage: `nixgraph <store path> --depth=<N>` for runtime deps; use `--buildtime` for build dependencies.
    Depth is default 1.
    You can also put in flake references `.#pkgname`.
  - `replology_cli`
  - `sbomnix` can generate machine-readable bill-of-materials, with lisences and everything.
  - I haven't played with `vulnxscan`.
- `nix-tree` is quite fast, and I like the interaction.
  It doesn't give a whoole lot, necesarily, but it does have a `--dot` flag.

References:
- [`nix-store -q`](https://nix.dev/manual/nix/latest/command-ref/nix-store/query) : built-in to nix
- [nix-visualize](https://github.com/craigmbooth/nix-visualize): visualize results of `nix-store -q --graph` queries
- [sbomnix](https://github.com/tiiuae/sbomnix): create sboms, visualize dependency graphs, vulnerability scanning, and so on
- [nix2sbom](https://github.com/louib/nix2sbom): alternative SBOM maker

Development Environments
========================

Minimize the environoment
-------------------------

Esp b/c it could speed up/shrink CI/CD infrastructure.

- [ ] follow up on [blog](https://fzakaria.com/2021/08/02/a-minimal-nix-shell.html)

Set `sh` to a posix shell
-------------------------

Normally, nixpkgs' shell links `/bin/sh` to `/bin/bash`, which isn't great for integration testing.
I'm not sure how to do it right, but I have some leads:
- [x] [nix.dev](https://nix.dev/tutorials/callpackage) has some good info about `callPackage`/`.override`.
  Nope: I can't actually get to the derivation to modify it.
- [ ] nixos & flakes book](https://nixos-and-flakes.thiscute.world/nixpkgs/overriding) has some stuff, but it's not immediately clear w/o prior knowledge
- [ ] [this lilne](https://github.com/NixOS/nixpkgs/blob/4e78fb68f65c9bcdf9543cdd01b35c911bf5d283/pkgs/shells/bash/5.nix#L155) specifies the symlink.
  What I don't like is that there's no input attr to the package that lets me disable it.
  - [x] So, what if I just overrode the `postInstall` to remove a symlink if it gets created?
    I haven't been able to get to the `postInstall` attr.
  - [x] copy the bash package out of nixpkgs and build a modified version myself?
    It won't even build unmodified.
  - [ ] What if I forked that bash-building code to add such an attribute (and then submitted a PR?)
  - [x] SUCCESS: `overrideAttrs` on a package gets me access to the derivation!U

It means rebuilding bash (and your posix shell of choice, but consider the following code.
This creates new packages (things you can pass where you would pass to `buildInputs`&c) from the nixpkgs repo.
The first binding causes the `dash` repo to link its own exe to `sh`; the second causes the `bash` package to remove its link (assuming it created one).
```
posixsh = pkgs.dash.overrideAttrs (final: prev: {
    postInstall = (prev.postInstall or "") + ''
    ln -s $out/bin/dash $out/bin/sh
    '';
});
bash_noln = pkgs.bash.overrideAttrs (final: prev: {
    postInstall = (prev.postInstall or "") + ''
    rm $out/bin/sh
    '';
});
```

References:
- [nix manual on overrideAttrs](https://ryantm.github.io/nixpkgs/using/overrides/)
- nix discorse on overrideAttrs](https://discourse.nixos.org/t/postinstall-or-hook/33994/5)
