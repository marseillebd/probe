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

