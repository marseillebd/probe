{
  description = "build hellos in some languages to demo nix for development";
  # to update all inputs:
  # $ nix flake update --recreate-lock-file

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/master"; # the lock file will have whatever was latest until an update is done
    flake-utils.url = "github:numtide/flake-utils"; # this has some really handy tools; a defacto stdlib
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }: flake-utils.lib.eachSystem
    [ "x86_64-linux"
      "aarch64-linux"
    ]
    (system:
    let
      pkgs = import nixpkgs {
        # here we configure the specific view of the packages database we want
        inherit system; # rn, I just want the target system set
      };
    # `rec` I've heard is not recommended, but since `self.packages.<X>` won't work, I use it here
    # all the member names for the record are in scope of a rec attrset
    # it's nice to use to set default package/devshell, maybe also sharing dependencies
    in rec {

      # for `nix run`, `nix build`
      packages.default = packages.liba;

      devShells.default = pkgs.mkShell {
        packages =  packages.liba.nativeBuildInputs
                 ++ packages.liba.buildInputs;
      };

      packages.liba = pkgs.stdenv.mkDerivation {
        pname = "liba";
        version = "0.0.1";
        src = ./liba;

        buildPhase = ''
          # code binaries
          bash scripts/compile.bash

          # documentation
          bash scripts/docs.bash <src/a.h >build/liba.md
          bash scripts/docs.bash <src/a.c >build/liba-devel.md
        '';
        installPhase = ''
          # actual install files: header, static and shared libraries
          mkdir -p $out/include
          cp src/a.h $out/include/
          mkdir -p $out/lib
          cp build/liba.a $out/lib/

          # DELETEME, the debug executable
          mkdir -p $out/bin
          cp build/a $out/bin/liba

          # documentation
          mkdir -p $out/docs
          cp build/liba.md $out/docs/
          cp build/liba-devel.md $out/docs/

          # create a single-file-header distribution
          mkdir -p $out/dist/stb
          cat src/a.h \
              <(printf "\n#ifdef A_IMPLEMENTATION\n\n") \
              src/a.c \
              <(printf "\n\n#endif\n") \
              > $out/dist/stb/a.h
        '';

        nativeBuildInputs = [];
        buildInputs = [];
      };

    });
}
