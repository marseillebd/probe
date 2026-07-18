{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils"; # this has some really handy tools; a defacto stdlib
  };

  outputs = { self, nixpkgs, flake-utils, }: flake-utils.lib.eachSystem
  [ "x86_64-linux"
  "aarch64-linux"
  ] (system:
  let
    pkgs = import nixpkgs { inherit system; };
  in {

    packages.default = pkgs.stdenv.mkDerivation {
      pname = "mar-sysinfo";
      version = "0.0";
      src = ./.;

      buildPhase = ''
        mkdir -p $out/
        cp ./mar-sysinfo.bash $out/
        chmod +x $out/mar-sysinfo.bash
      '';
      installPhase = ''
        mkdir -p $out/bin
        cp ./mar-sysinfo.bash $out/bin/mar-sysinfo
      '';
      buildInputs = with pkgs; [
        util-linux
        #^ ls{cpu,mem,blk,clocks,logins}
      ];
    };

  });
}
