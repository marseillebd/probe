{
  description = "The top-level flake for Marseille Bouchard's probe (aka experiment) repository.";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-25.05";
    flake-utils.url = "github:numtide/flake-utils";
    nixpkgs-unstable.url = "github:nixos/nixpkgs?ref=nixos-unstable";
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

    tooling.shells = with pkgs; [
      bash       # interpreter
      shellcheck # linter
      shfmt      # formatter
    ];
  in

    {
      devShells.default = pkgs.mkShell {
        nativeBuildInputs = tooling.shells;
      };
    }

  );
}
