{
  description = "a toy compiler for a staged, statically linearly- and dependently-typed, pure functional programming language";
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
      packages.default = packages.tdc;
      # for `nix develop`
      devShells.default = devShells.tdc;

      devShells.tdc = pkgs.mkShell {
        buildInputs =
          packages.tdc.nativeBuildInputs
          ++ packages.tdc.buildInputs;
      };

      packages.tdc = pkgs.stdenv.mkDerivation {
        pname = "tdc";
        version = "0.0";
        src = ./.;

        buildPhase = ''
          . scripts/compile.bash
          buildExe tdc # this is broken, because cabal is desperate for $HOME and network access and who knows what else?
          buildDocs index
        '';
        installPhase = ''
          # executable
          mkdir -p $out/bin
          mv build/tdc $out/bin/
          # documentation
          mkdir -p $out/docs
          cp docs/*.md $out/docs/
          cp build/*.html $out/docs/
        '';
        nativeBuildInputs = with pkgs; [
          pandoc
          haskell.compiler.ghc910 cabal-install
          qbe
        ];
        buildInputs = [];
      };

      packages.smoke = pkgs.stdenv.mkDerivation {
        pname = "smoke";
        version = "0.0";
        src = ./.;
        nativeBuildInputs = [
          packages.tdc
        ];
        buildPhase = ''
          . scripts/compile.bash
          buildExe tdc
        '';
        installPhase = ''
          mkdir -p $out/bin
          echo >$out/bin/smoke \
            '#!/usr/bin/env bash
            echo OK'
          chmod +x $out/bin/smoke
        '';
      };

    });
}
