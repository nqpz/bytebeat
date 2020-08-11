with import <nixpkgs> {};
stdenv.mkDerivation {
    name = "lys";
    buildInputs = [ pkgconfig tinycc ];
}
