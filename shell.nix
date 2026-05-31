# default.nix
with import <nixpkgs> {}; {
projectEnv = stdenv.mkDerivation {
name = "projectName";
buildInputs = [stdenv ncurses];
propagateBuildInputs = [ncurses];
};
}
