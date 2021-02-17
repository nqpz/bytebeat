with import <nixpkgs> {};

mkShell {
  buildInputs = [ pkgconfig tinycc sox ];
}
