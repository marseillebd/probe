{
  description = "The top-level flake for Marseille Bouchard's probe (aka experiment) repository.";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-25.05";
    nixpkgs-unstable.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    ...
  } @ args:
  flake-utils.lib.eachDefaultSystem (system:
  let
    pkgs = import nixpkgs { inherit system; };
    lib = pkgs.lib;

    slimcc = pkgs.stdenv.mkDerivation {
      name = "slimcc";
      src = pkgs.fetchFromGitHub {
        # https://github.com/fuhsnn/slimcc
        owner = "fuhsnn";
        repo = "slimcc";
        rev = "4c1711d453d566c63fcde7c3bac7c598e25e8071"; # 5 Jan 2026
        hash = "sha256-uVq9V5hVzCQ51XXonPhRL+ZP8JoH81KrbFxq+eayO7Y=";
      };
      configurePhase = ''
        ln -s platform/linux-glibc-generic.c platform.c
      '';
      installPhase = ''
        mkdir -p "$out/bin"
        mv slimcc "$out/bin/slimcc"
      '';
    };

    # These two bindings create modified versions of existing packages.
    # Specifically, `bash` no longer tries to link itself into `sh`,
    # and we get a posix-compliant shell (dash) to link itself to `sh` instead.
    # TODO what if I made a package where you can just set the package you want to symlink to?
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

    tooling.plaintext = with pkgs; [
      eclint
    ];

    tooling.shells = with pkgs; [
      posixsh    # sh interpreter (links to dash)
      bash_noln  # bash interpreter
      shellcheck # linter
      shfmt      # formatter
    ];

    tooling.c = with pkgs; [
      gcc15 # even the latest (Jan 2026) gcc doesn't have the defer patch shipped
      # it might be getting close, thouogh: https://gcc.gnu.org/pipermail/gcc-patches/2025-August/691465.html
      clang # clang won't have defer until v22, which isn't out yet (Jan 2026); also, I can't figure out how to get the man pages, even with clang-manpages
      # slimcc # I'm not sure how to get this to work in nix
    ];

  in
    {

      devShells.default = pkgs.mkShellNoCC {
        nativeBuildInputs
          = tooling.plaintext
          ++ tooling.shells
          ++ tooling.c
        ;
      };

    }

  );
}
