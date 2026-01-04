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

    # TODO extract this to a package, and also see if it works regardless of package include order
    posixsh = pkgs.stdenvNoCC.mkDerivation {
      name = "symlink-sh-dash";
      dontUnpack = true;
      buildInputs = [pkgs.dash pkgs.which];
      buildPhase = "which dash >&2";
      installPhase = ''
        mkdir -p $out/bin
        ln -s "$(which dash)" "$out/bin/sh"
      '';
    };

    tooling.shells = with pkgs; [
      posixsh    # interpreter (links to dash)
      bash       # interpreter
      shellcheck # linter
      shfmt      # formatter
    ];
  in

    {

      devShells.default = pkgs.mkShellNoCC {
        nativeBuildInputs = tooling.shells;
      };

    }

  );
}
