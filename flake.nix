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

    tooling.utils = with pkgs; [
      silver-searcher ripgrep # TODO evaluate which I like better
      icdiff
      inotify-tools
      moreutils
      rlwrap
      tree
    ];

    tooling.shells = with pkgs; [
      posixsh    # sh interpreter (links to dash)
      bash_noln  # bash interpreter
      shellcheck # linter
      shfmt      # formatter
    ];

  in
    {

      devShells.default = pkgs.mkShellNoCC {
        nativeBuildInputs =
            tooling.utils
          ++ tooling.shells;
      };

    }

  );
}
