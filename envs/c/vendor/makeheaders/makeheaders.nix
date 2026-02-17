{ stdenv, fetchurl, ...}:
stdenv.mkDerivation {
  name = "makeheaders";
  src = fetchurl {
    url = "https://www2.fossil-scm.org/fossil/raw/2d4ffaf9812b93c894010ef9071b08af0d1df2e0d947d8d35a192d24ea16bf0b?at=makeheaders.c";
    hash = "sha256-/gZ0AQ+6mw1jhZJc4J6/5idWSXSWfOgieGdjfkNPINc=";
  };
  unpackPhase = ''
    cp "$src" "''${src##*?at=}"
  '';
  buildPhase = ''
    gcc makeheaders.c -o makeheaders
  '';
  installPhase = ''
    mkdir -p $out/bin
    cp makeheaders $out/bin
  '';
}
